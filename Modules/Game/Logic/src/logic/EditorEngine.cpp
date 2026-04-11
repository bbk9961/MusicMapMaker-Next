#include "logic/EditorEngine.h"
#include "config/AppConfig.h"
#include "event/canvas/interactive/ResizeEvent.h"
#include "event/core/EventBus.h"
#include "event/logic/EditorConfigChangedEvent.h"
#include "event/logic/LogicCommandEvent.h"
#include "event/ui/menu/OpenProjectEvent.h"
#include "event/ui/menu/ProjectLoadedEvent.h"
#include "log/colorful-log.h"
#include "logic/BeatmapSyncBuffer.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace MMM::Logic
{

EditorEngine& EditorEngine::instance()
{
    static EditorEngine instance;
    return instance;
}

EditorEngine::EditorEngine()
{
    // 从全局配置初始化本地缓存
    m_editorConfig = Config::AppConfig::instance().getEditorConfig();

    // 默认创建一个 Session
    m_activeSession = std::make_unique<BeatmapSession>();

    // 订阅画布尺寸改变事件
    Event::EventBus::instance().subscribe<Event::CanvasResizeEvent>(
        [this](const Event::CanvasResizeEvent& e) {
            pushCommand(CmdUpdateViewport{ e.canvasName,
                                           static_cast<float>(e.newSize.x),
                                           static_cast<float>(e.newSize.y) });
        });

    // 订阅打开项目事件
    Event::EventBus::instance().subscribe<Event::OpenProjectEvent>(
        [this](const Event::OpenProjectEvent& e) {
            openProject(e.m_projectPath);
        });

    // 订阅逻辑指令事件
    Event::EventBus::instance().subscribe<Event::LogicCommandEvent>(
        [this](const Event::LogicCommandEvent& e) {
            if ( std::holds_alternative<CmdUpdateEditorConfig>(e.command) ) {
                setEditorConfig(
                    std::get<CmdUpdateEditorConfig>(e.command).config);
            } else {
                pushCommand(MMM::Logic::LogicCommand(e.command));
            }
        });
}

EditorEngine::~EditorEngine()
{
    stop();
}

void EditorEngine::openProject(const std::filesystem::path& projectPath)
{
    if ( !std::filesystem::exists(projectPath) ||
         !std::filesystem::is_directory(projectPath) ) {
        XERROR(
            "Failed to open project: Path does not exist or is not a "
            "directory: {}",
            projectPath.string());
        return;
    }

    XINFO("Opening project at: {}", projectPath.string());

    auto newProject           = std::make_unique<Project>();
    newProject->m_projectRoot = projectPath;
    newProject->m_metadata.m_title =
        projectPath.filename().string();  // 默认标题为目录名

    // 扫描文件系统
    try {
        for ( const auto& entry :
              std::filesystem::recursive_directory_iterator(projectPath) ) {
            if ( !entry.is_regular_file() ) continue;

            auto ext = entry.path().extension().string();
            auto relPath =
                std::filesystem::relative(entry.path(), projectPath).string();
            auto filename = entry.path().filename().string();

            // 1. 扫描音频 (作为主轨道)
            if ( ext == ".mp3" || ext == ".ogg" || ext == ".wav" ||
                 ext == ".flac" ) {
                AudioResource res;
                res.m_id     = filename;
                res.m_path   = relPath;
                res.m_type   = AudioTrackType::Main;
                res.m_volume = 1.0f;
                newProject->m_audioResources.push_back(res);
                XINFO("Found audio resource: {}", filename);
            }

            // 2. 扫描谱面 (*.osu, *.imd, *.mc)
            if ( ext == ".osu" || ext == ".imd" || ext == ".mc" ) {
                Project::BeatmapEntry mapEntry;
                mapEntry.m_name     = filename;
                mapEntry.m_filePath = relPath;
                // 默认策略：关联当前已找到的第一个音轨
                if ( !newProject->m_audioResources.empty() ) {
                    mapEntry.m_audioTrackId =
                        newProject->m_audioResources.front().m_id;
                }
                newProject->m_beatmaps.push_back(mapEntry);
                XINFO("Found beatmap: {}", filename);
            }
        }
    } catch ( const std::exception& e ) {
        XERROR("Error while scanning project directory: {}", e.what());
    }

    // 检查是否有项目描述文件
    std::filesystem::path projectFile = projectPath / "mmm_project.json";
    if ( std::filesystem::exists(projectFile) ) {
        try {
            std::ifstream  file(projectFile);
            nlohmann::json j;
            file >> j;
            Project loadedProject  = j.get<Project>();
            newProject->m_metadata = loadedProject.m_metadata;
            newProject->m_settings = loadedProject.m_settings;
            XINFO("Project configuration loaded from mmm_project.json");
        } catch ( ... ) {
            XWARN(
                "Failed to load existing mmm_project.json, using scanned "
                "results.");
        }
    }

    // 自动持久化扫描结果 (标记此目录为项目)
    try {
        std::ofstream  file(projectFile);
        nlohmann::json j = *newProject;
        file << std::setw(4) << j << std::endl;
    } catch ( ... ) {
    }

    // 更新当前项目单例状态
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        m_currentProject = std::move(newProject);
    }

    // 发布加载成功事件
    Event::ProjectLoadedEvent loadedEv;
    loadedEv.m_projectTitle = m_currentProject->m_metadata.m_title;
    loadedEv.m_projectPath  = projectPath.string();
    loadedEv.m_beatmapCount = m_currentProject->m_beatmaps.size();
    Event::EventBus::instance().publish(loadedEv);

    XINFO("Project '{}' loaded successfully with {} beatmaps.",
          loadedEv.m_projectTitle,
          loadedEv.m_beatmapCount);

    // 记录到最近打开列表
    Config::AppConfig::instance().addRecentProject(projectPath.string());
    m_editorConfig.recentProjects =
        Config::AppConfig::instance().getEditorConfig().recentProjects;

    // 新加载项目时，清空当前 Session 的所有谱面数据
    pushCommand(CmdLoadBeatmap{ nullptr });
}

void EditorEngine::start()
{
    if ( m_running ) {
        return;
    }

    // 从全局配置同步到本地缓存
    m_editorConfig = Config::AppConfig::instance().getEditorConfig();

    m_running = true;

    m_thread = std::thread(&EditorEngine::loop, this);
    XINFO("EditorEngine logic thread started.");
}

void EditorEngine::stop()
{
    if ( m_running ) {
        m_running = false;
        if ( m_thread.joinable() ) {
            m_thread.join();
        }
        XINFO("EditorEngine logic thread stopped.");
    }
}

void EditorEngine::pushCommand(LogicCommand&& cmd)
{
    if ( m_activeSession ) {
        m_activeSession->pushCommand(std::move(cmd));
    }
}

std::shared_ptr<BeatmapSyncBuffer> EditorEngine::getSyncBuffer(
    const std::string& cameraId)
{
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    if ( m_syncBuffers.find(cameraId) == m_syncBuffers.end() ) {
        m_syncBuffers[cameraId] = std::make_shared<BeatmapSyncBuffer>();
    }
    return m_syncBuffers[cameraId];
}

EditTool EditorEngine::getCurrentTool() const
{
    if ( m_activeSession ) {
        return m_activeSession->getCurrentTool();
    }
    return EditTool::Move;
}

void EditorEngine::setEditorConfig(const Config::EditorConfig& config)
{
    // 关键修复：从全局 AppConfig 中同步最新的最近项目列表，防止被 UI 设置覆盖
    auto& globalRecent =
        Config::AppConfig::instance().getEditorConfig().recentProjects;

    m_editorConfig                = config;
    m_editorConfig.recentProjects = globalRecent;

    // 同步回全局 AppConfig 实例
    Config::AppConfig::instance().getEditorConfig() = m_editorConfig;

    pushCommand(CmdUpdateEditorConfig{ m_editorConfig });

    // 发布配置更新事件，供 UI 层订阅
    Event::EventBus::instance().publish(
        Event::EditorConfigChangedEvent{ m_editorConfig });
}

void EditorEngine::loop()
{
    auto lastTime = std::chrono::high_resolution_clock::now();
    // 限制逻辑线程最高帧率约为 240Hz (4.16ms)
    // 这能确保 Logic 线程拥有极高的平滑度且不会100%占用CPU核心引发调度抖动
    const double targetDt = 1.0 / 240.0;

    while ( m_running ) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> passed = currentTime - lastTime;

        // 如果距离上一帧还没有达到 1/240 秒，就主动让出 CPU
        if ( passed.count() < targetDt ) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        lastTime  = currentTime;
        double dt = passed.count();

        if ( m_activeSession ) {
            m_activeSession->update(dt, m_editorConfig);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

}  // namespace MMM::Logic

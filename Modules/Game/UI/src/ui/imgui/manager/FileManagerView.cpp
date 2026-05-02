#include "ui/imgui/manager/FileManagerView.h"
#include "config/skin/SkinConfig.h"
#include "event/core/EventBus.h"
#include "event/input/glfw/GLFWDropEvent.h"
#include "imgui.h"
#include "logic/EditorEngine.h"
#include <filesystem>

namespace MMM::UI
{

FileManagerView::FileManagerView(const std::string& subViewName)
    : ISubView(subViewName)
{
    m_currentRoot = std::filesystem::current_path();

    m_dropSubId = Event::EventBus::instance().subscribe<Event::GLFWDropEvent>(
        [this](const Event::GLFWDropEvent& e) {
            m_pendingDrops.push_back({ e.paths, e.pos });
        });
}

FileManagerView::~FileManagerView()
{
    Event::EventBus::instance().unsubscribe<Event::GLFWDropEvent>(m_dropSubId);
}

void FileManagerView::onUpdate(LayoutContext& layoutContext,
                               UIManager*     sourceManager)
{
    // 1. 处理拖拽
    handleDragDrop(sourceManager);

    auto& engine  = Logic::EditorEngine::instance();
    auto* project = engine.getCurrentProject();
    auto& skinCfg = Config::SkinManager::instance();

    ImFont* fileManagerFont = skinCfg.getFont("filemanager");
    if ( fileManagerFont ) ImGui::PushFont(fileManagerFont);

    if ( !project ) {
        renderEmptyProjectView(layoutContext);
    } else {
        renderActiveProjectView(layoutContext, sourceManager);
    }

    if ( fileManagerFont ) ImGui::PopFont();
}

} // namespace MMM::UI

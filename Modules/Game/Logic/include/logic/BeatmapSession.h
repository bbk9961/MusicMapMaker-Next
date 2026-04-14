#pragma once

#include "common/LogicCommands.h"
#include <concurrentqueue.h>
#include <memory>

namespace MMM::Config { struct EditorConfig; }
namespace MMM::Logic {

struct SessionContext;
class PlaybackController;
class InteractionController;
class ActionController;

/**
 * @brief 谱面逻辑会话核心 (Facade / Controller Manager)
 *
 * 持有各子系统和共享上下文，处理逻辑线程中的业务更新。
 */
class BeatmapSession
{
public:
    BeatmapSession();
    ~BeatmapSession();

    // 禁用拷贝与移动
    BeatmapSession(BeatmapSession&&)                 = delete;
    BeatmapSession(const BeatmapSession&)            = delete;
    BeatmapSession& operator=(BeatmapSession&&)      = delete;
    BeatmapSession& operator=(const BeatmapSession&) = delete;

    /**
     * @brief 推送指令到无锁队列（由 UI 线程调用）
     */
    void pushCommand(LogicCommand&& cmd);

    /**
     * @brief 每帧更新（由 Logic 线程调用）
     * @param dt 距离上一帧的时间间隔（秒）
     * @param config 当前编辑器配置
     */
    void update(double dt, const Config::EditorConfig& config);

    /**
     * @brief 获取共享上下文的只读引用（供 UI 渲染层读取）
     */
    const SessionContext& getContext() const { return *m_ctx; }
    
    /**
     * @brief 供需要完全访问的系统获取
     */
    SessionContext& getContextMutable() { return *m_ctx; }

private:
    void processCommands();
    void updateECSAndRender(const Config::EditorConfig& config);

    void handleCommand(const CmdUpdateEditorConfig& cmd);
    void handleCommand(const CmdUpdateViewport& cmd);
    void handleCommand(const CmdLoadBeatmap& cmd);
    void handleCommand(const CmdSaveBeatmap& cmd);
    void handleCommand(const CmdPackBeatmap& cmd);

    std::unique_ptr<SessionContext> m_ctx;
    std::unique_ptr<PlaybackController> m_playback;
    std::unique_ptr<InteractionController> m_interaction;
    std::unique_ptr<ActionController> m_actions;

    moodycamel::ConcurrentQueue<LogicCommand> m_commandQueue;
};

}  // namespace MMM::Logic

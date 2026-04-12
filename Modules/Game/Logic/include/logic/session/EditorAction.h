#pragma once

#include <memory>
#include <vector>

namespace MMM::Logic
{

class BeatmapSession;

/**
 * @brief 编辑操作接口
 */
class IEditorAction
{
public:
    virtual ~IEditorAction() = default;

    /**
     * @brief 执行操作 (初次执行)
     */
    virtual void execute(BeatmapSession& session) = 0;

    /**
     * @brief 撤销操作
     */
    virtual void undo(BeatmapSession& session) = 0;

    /**
     * @brief 重做操作
     */
    virtual void redo(BeatmapSession& session) = 0;
};

/**
 * @brief 操作栈管理器
 */
class EditorActionStack
{
public:
    /**
     * @brief 执行并推送新操作到栈中，同时清空重做栈
     */
    void pushAndExecute(std::unique_ptr<IEditorAction> action, BeatmapSession& session)
    {
        action->execute(session);
        m_undoStack.push_back(std::move(action));
        m_redoStack.clear();
    }

    /**
     * @brief 撤销
     */
    void undo(BeatmapSession& session)
    {
        if ( m_undoStack.empty() ) return;
        auto action = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        action->undo(session);
        m_redoStack.push_back(std::move(action));
    }

    /**
     * @brief 重做
     */
    void redo(BeatmapSession& session)
    {
        if ( m_redoStack.empty() ) return;
        auto action = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        action->redo(session);
        m_undoStack.push_back(std::move(action));
    }

    void clear()
    {
        m_undoStack.clear();
        m_redoStack.clear();
    }

private:
    std::vector<std::unique_ptr<IEditorAction>> m_undoStack;
    std::vector<std::unique_ptr<IEditorAction>> m_redoStack;
};

} // namespace MMM::Logic

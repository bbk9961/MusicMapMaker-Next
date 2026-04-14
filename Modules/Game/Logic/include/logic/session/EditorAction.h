#pragma once

#include <memory>
#include <vector>

namespace MMM::Logic
{

struct SessionContext;

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
    virtual void execute(SessionContext& ctx) = 0;

    /**
     * @brief 撤销操作
     */
    virtual void undo(SessionContext& ctx) = 0;

    /**
     * @brief 重做操作
     */
    virtual void redo(SessionContext& ctx) = 0;
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
    void pushAndExecute(std::unique_ptr<IEditorAction> action,
                        SessionContext&                ctx);

    /**
     * @brief 撤销
     */
    void undo(SessionContext& ctx);

    /**
     * @brief 重做
     */
    void redo(SessionContext& ctx);

    void clear();

private:
    std::vector<std::unique_ptr<IEditorAction>> m_undoStack;
    std::vector<std::unique_ptr<IEditorAction>> m_redoStack;
};

}  // namespace MMM::Logic

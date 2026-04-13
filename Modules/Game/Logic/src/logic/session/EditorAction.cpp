#include "logic/session/EditorAction.h"
#include "logic/BeatmapSession.h"

namespace MMM::Logic
{

void EditorActionStack::pushAndExecute(std::unique_ptr<IEditorAction> action,
                                       BeatmapSession&                session)
{
    action->execute(session);
    m_undoStack.push_back(std::move(action));
    m_redoStack.clear();
    session.syncBeatmap();
}

void EditorActionStack::undo(BeatmapSession& session)
{
    if ( m_undoStack.empty() ) return;
    auto action = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    action->undo(session);
    m_redoStack.push_back(std::move(action));
    session.syncBeatmap();
}

void EditorActionStack::redo(BeatmapSession& session)
{
    if ( m_redoStack.empty() ) return;
    auto action = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    action->redo(session);
    m_undoStack.push_back(std::move(action));
    session.syncBeatmap();
}

void EditorActionStack::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

}  // namespace MMM::Logic

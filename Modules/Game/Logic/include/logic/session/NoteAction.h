#pragma once

#include "logic/session/EditorAction.h"
#include "logic/ecs/components/NoteComponent.h"
#include <entt/entt.hpp>
#include <optional>

namespace MMM::Logic
{

/**
 * @brief 音符操作 (增/删/改)
 */
class NoteAction : public IEditorAction
{
public:
    enum class Type { Update, Create, Delete };

    NoteAction(Type type, entt::entity entity,
               std::optional<NoteComponent> before,
               std::optional<NoteComponent> after)
        : m_type(type), m_entity(entity), m_before(before), m_after(after) {}

    void execute(BeatmapSession& session) override;
    void undo(BeatmapSession& session) override;
    void redo(BeatmapSession& session) override;

private:
    Type m_type;
    entt::entity m_entity;
    std::optional<NoteComponent> m_before;
    std::optional<NoteComponent> m_after;
};

} // namespace MMM::Logic

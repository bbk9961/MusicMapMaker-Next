#pragma once

#include "logic/session/EditorAction.h"
#include "logic/ecs/components/TimelineComponent.h"
#include <entt/entt.hpp>
#include <optional>

namespace MMM::Logic
{

/**
 * @brief 时间线事件操作 (增/删/改)
 */
class TimelineAction : public IEditorAction
{
public:
    enum class Type { Create, Delete, Update };

    TimelineAction(Type type, entt::entity entity, 
                   std::optional<TimelineComponent> before, 
                   std::optional<TimelineComponent> after)
        : m_type(type), m_entity(entity), m_before(before), m_after(after) {}

    void execute(BeatmapSession& session) override;
    void undo(BeatmapSession& session) override;
    void redo(BeatmapSession& session) override;

private:
    Type m_type;
    entt::entity m_entity;
    std::optional<TimelineComponent> m_before;
    std::optional<TimelineComponent> m_after;
};

} // namespace MMM::Logic

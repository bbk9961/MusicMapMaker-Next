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

    void execute(SessionContext& ctx) override;
    void undo(SessionContext& ctx) override;
    void redo(SessionContext& ctx) override;

private:
    Type m_type;
    entt::entity m_entity;
    std::optional<TimelineComponent> m_before;
    std::optional<TimelineComponent> m_after;
};

} // namespace MMM::Logic

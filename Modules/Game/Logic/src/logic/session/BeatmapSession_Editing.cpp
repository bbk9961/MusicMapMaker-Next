#include "logic/BeatmapSession.h"
#include "logic/ecs/components/TimelineComponent.h"
#include "logic/session/TimelineAction.h"

namespace MMM::Logic
{

// --- Editing Handlers ---

void BeatmapSession::handleCommand(const CmdUndo& cmd)
{
    m_actionStack.undo(*this);
}

void BeatmapSession::handleCommand(const CmdRedo& cmd)
{
    m_actionStack.redo(*this);
}

void BeatmapSession::handleCommand(const CmdCopy& cmd) {}
void BeatmapSession::handleCommand(const CmdPaste& cmd) {}
void BeatmapSession::handleCommand(const CmdCut& cmd) {}

// --- Timeline Handlers ---

void BeatmapSession::handleCommand(const CmdUpdateTimelineEvent& cmd)
{
    if ( m_timelineRegistry.valid(cmd.entity) ) {
        auto oldTl = m_timelineRegistry.get<TimelineComponent>(cmd.entity);
        auto newTl = oldTl;
        newTl.m_timestamp = cmd.newTime;
        newTl.m_value     = cmd.newValue;

        auto action = std::make_unique<TimelineAction>(
            TimelineAction::Type::Update, cmd.entity, oldTl, newTl);
        m_actionStack.pushAndExecute(std::move(action), *this);
    }
}

void BeatmapSession::handleCommand(const CmdDeleteTimelineEvent& cmd)
{
    if ( m_timelineRegistry.valid(cmd.entity) ) {
        auto oldTl  = m_timelineRegistry.get<TimelineComponent>(cmd.entity);
        auto action = std::make_unique<TimelineAction>(
            TimelineAction::Type::Delete, cmd.entity, oldTl, std::nullopt);
        m_actionStack.pushAndExecute(std::move(action), *this);
    }
}

void BeatmapSession::handleCommand(const CmdCreateTimelineEvent& cmd)
{
    TimelineComponent newTl{ cmd.time, cmd.type, cmd.value };
    auto              action = std::make_unique<TimelineAction>(
        TimelineAction::Type::Create, entt::null, std::nullopt, newTl);
    m_actionStack.pushAndExecute(std::move(action), *this);
}

} // namespace MMM::Logic

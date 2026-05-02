#pragma once

#include "common/LogicCommands.h"
#include "event/EventDef.h"
#include "event/core/BaseEvent.h"

namespace MMM::Event
{

/**
 * @brief 逻辑指令事件，用于在 UI/Canvas 与 Logic 之间解耦传递指令。
 */
struct LogicCommandEvent : public BaseEvent {
    MMM::Logic::LogicCommand command;

    LogicCommandEvent(const MMM::Logic::LogicCommand& cmd) : command(cmd) {}
    LogicCommandEvent(MMM::Logic::LogicCommand&& cmd) : command(std::move(cmd))
    {
    }
};

}  // namespace MMM::Event

EVENT_REGISTER_PARENTS(MMM::Event::LogicCommandEvent, MMM::Event::BaseEvent)

#pragma once

#include "config/EditorConfig.h"
#include "event/EventDef.h"
#include "event/core/BaseEvent.h"

namespace MMM::Event
{

/**
 * @brief 编辑器配置变更事件，当 Logic 层修改了配置（如通过
 * SettingsView）时，发布此事件供 UI 层更新。
 */
struct EditorConfigChangedEvent : public BaseEvent {
    MMM::Config::EditorConfig config;

    EditorConfigChangedEvent(const MMM::Config::EditorConfig& cfg) : config(cfg)
    {
    }
};

}  // namespace MMM::Event

EVENT_REGISTER_PARENTS(MMM::Event::EditorConfigChangedEvent,
                       MMM::Event::BaseEvent)

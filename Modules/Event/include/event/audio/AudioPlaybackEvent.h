#pragma once

#include "event/core/BaseEvent.h"

namespace MMM::Event
{

/**
 * @brief 音频播放完成事件
 */
struct AudioFinishedEvent : public BaseEvent {
    bool isLooping;
};

/**
 * @brief 音频播放实时位置更新事件 (非连续，按缓冲区发送)
 */
struct AudioPositionEvent : public BaseEvent {
    double positionSeconds;
    double
        systemTimeSeconds;  // 发送事件时的系统时间，用于消除事件队列传递带来的抖动
};

}  // namespace MMM::Event

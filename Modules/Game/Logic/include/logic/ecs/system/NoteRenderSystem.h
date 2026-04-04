#pragma once

#include "logic/BeatmapSyncBuffer.h"
#include <entt/entt.hpp>

namespace MMM::Logic::System
{

/**
 * @brief 音符渲染快照生成系统
 *
 * 将 ECS 逻辑坐标转换并剔除到视口快照 (RenderSnapshot) 中供 UI 线程渲染。
 */
class NoteRenderSystem
{
public:
    /**
     * @brief 生成快照
     *
     * @param registry 音符注册表
     * @param timelineRegistry 时间线注册表 (用于坐标积分映射)
     * @param snapshot 目标渲染快照缓冲区
     * @param currentTime 当前播放时间
     * @param viewportHeight 视口总高度
     * @param judgmentLineY 判定线位置 (视口空间)
     */
    static void generateSnapshot(entt::registry&       registry,
                                 const entt::registry& timelineRegistry,
                                 RenderSnapshot* snapshot, double currentTime,
                                 float viewportHeight, float judgmentLineY);
};

}  // namespace MMM::Logic::System

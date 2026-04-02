#pragma once

#include "logic/BeatmapSyncBuffer.h"
#include <entt/entt.hpp>

namespace MMM::Logic::System
{

/**
 * @brief 音符位置计算系统
 *
 * 负责根据当前时间和流速计算每个音符的相对坐标 (Y 轴)。
 */
class NoteTransformSystem
{
public:
    /**
     * @brief 更新所有音符的变换组件
     *
     * @param registry 音符注册表
     * @param timelineRegistry 时间线注册表 (用于查流速)
     * @param currentTime 当前播放时间
     * @param judgmentLineY 判定线的 Y 坐标 (屏幕空间)
     */
    static void update(entt::registry&       registry,
                       const entt::registry& timelineRegistry,
                       double currentTime, float judgmentLineY);
};

/**
 * @brief 音符渲染快照生成系统
 *
 * 负责将含有位置的音符转换为顶点缓冲并填入 RenderSnapshot 中，同时进行剔除。
 */
class NoteRenderSystem
{
public:
    /**
     * @brief 生成快照
     *
     * @param registry 音符注册表
     * @param snapshot 目标快照缓冲区
     * @param viewportHeight 视口高度，用于上下边界剔除
     * @param judgmentLineY 摄像机空间的判定线高度
     */
    static void generateSnapshot(entt::registry& registry,
                                 RenderSnapshot* snapshot, float viewportHeight,
                                 float judgmentLineY);
};

}  // namespace MMM::Logic::System

#include "logic/BeatmapSession.h"
#include "log/colorful-log.h"
#include "logic/BeatmapSyncBuffer.h"
#include "logic/EditorEngine.h"
#include "logic/ecs/components/InteractionComponent.h"
#include "logic/ecs/components/NoteComponent.h"
#include "logic/ecs/components/TimelineComponent.h"
#include "logic/ecs/components/TransformComponent.h"
#include "logic/ecs/system/NoteSystem.h"
#include "mmm/beatmap/BeatMap.h"
#include <glm/glm.hpp>

namespace MMM::Logic
{

BeatmapSession::BeatmapSession() {}

void BeatmapSession::loadBeatmap(std::shared_ptr<MMM::BeatMap> beatmap)
{
    m_noteRegistry.clear();
    m_timelineRegistry.clear();

    m_isPlaying   = true;
    m_currentTime = 0.0;

    if ( !beatmap ) return;

    // 根据传入的 BeatMap 构建 ECS 实体
    for ( const auto& timing : beatmap->m_timings ) {
        auto entity = m_timelineRegistry.create();
        m_timelineRegistry.emplace<TimelineComponent>(
            entity,
            timing.m_timestamp,
            timing.m_timingEffect,
            timing.m_timingEffectParameter);
    }

    for ( size_t i = 0; i < beatmap->m_notes.size(); ++i ) {
        const auto& note   = beatmap->m_notes[i];
        auto        entity = m_noteRegistry.create();

        // 简单分配轨道
        int track = i % 4;

        m_noteRegistry.emplace<NoteComponent>(
            entity, note.m_type, note.m_timestamp, 0.0, track);

        m_noteRegistry.emplace<TransformComponent>(
            entity,
            glm::vec2(track * 60.0f + 20.0f,
                      -1000.0f),  // 初始位置无关紧要，会被更新
            glm::vec2(50.0f, 20.0f));
    }

    XINFO("Loaded new BeatMap with {} notes and {} timings.",
          beatmap->m_notes.size(),
          beatmap->m_timings.size());
}

void BeatmapSession::pushCommand(LogicCommand&& cmd)
{
    m_commandQueue.enqueue(std::move(cmd));
}

void BeatmapSession::update(double dt)
{
    processCommands();

    if ( m_isPlaying ) {
        m_currentTime += dt;
    }

    updateECSAndRender();
}

void BeatmapSession::processCommands()
{
    LogicCommand cmd;
    // 不断尝试出队直到队列为空
    while ( m_commandQueue.try_dequeue(cmd) ) {
        std::visit(
            [this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr ( std::is_same_v<T, CmdUpdateViewport> ) {
                    if ( m_cameras.find(arg.cameraId) == m_cameras.end() ) {
                        m_cameras[arg.cameraId] =
                            CameraInfo{ arg.cameraId, arg.width, arg.height };
                    } else {
                        m_cameras[arg.cameraId].viewportWidth  = arg.width;
                        m_cameras[arg.cameraId].viewportHeight = arg.height;
                    }
                } else if constexpr ( std::is_same_v<T, CmdSetPlayState> ) {
                    m_isPlaying = arg.isPlaying;
                } else if constexpr ( std::is_same_v<T, CmdLoadBeatmap> ) {
                    loadBeatmap(arg.beatmap);
                } else if constexpr ( std::is_same_v<T, CmdSetHoveredEntity> ) {
                    // 先清空所有实体的 Hover 状态 (简易处理，未来可优化)
                    auto view = m_noteRegistry.view<InteractionComponent>();
                    for ( auto entity : view ) {
                        m_noteRegistry.get<InteractionComponent>(entity)
                            .isHovered = false;
                    }

                    // 设置新的 Hover 实体
                    if ( arg.entity != entt::null ) {
                        // 如果没有该组件则添加
                        if ( !m_noteRegistry.all_of<InteractionComponent>(
                                 arg.entity) ) {
                            m_noteRegistry.emplace<InteractionComponent>(
                                arg.entity);
                        }
                        m_noteRegistry.get<InteractionComponent>(arg.entity)
                            .isHovered = true;
                    }
                }
            },
            cmd);
    }
}

void BeatmapSession::updateECSAndRender()
{
    // 1. 调用 ECS System 更新全局物理位置 (Logical Transform)
    // 注意：我们将 judgmentLineY 的计算延后到渲染阶段，所以这里的
    // TransformSystem 其实只计算逻辑距离 (relativeDistance)
    System::NoteTransformSystem::update(
        m_noteRegistry, m_timelineRegistry, m_currentTime, 0.0f);

    // 2. 遍历所有注册的视口 (Camera) 进行独立的视口剔除和坐标映射
    for ( const auto& [cameraId, camera] : m_cameras ) {
        // 从 EditorEngine 获取该 Camera 专属的缓冲
        auto syncBuffer = EditorEngine::instance().getSyncBuffer(cameraId);
        if ( !syncBuffer ) continue;

        RenderSnapshot* snapshot = syncBuffer->getWorkingSnapshot();
        if ( !snapshot ) continue;

        snapshot->clear();

        // 假设判定线在画布底部偏上 100 像素的位置
        float judgmentLineY = camera.viewportHeight - 100.0f;

        // 3. 调用 ECS System 针对当前 Camera 生成渲染快照
        System::NoteRenderSystem::generateSnapshot(
            m_noteRegistry, snapshot, camera.viewportHeight, judgmentLineY);

        // 4. 提交专属快照
        syncBuffer->pushWorkingSnapshot();
    }
}

}  // namespace MMM::Logic

#include "logic/ecs/system/NoteSystem.h"
#include "logic/ecs/components/InteractionComponent.h"
#include "logic/ecs/components/NoteComponent.h"
#include "logic/ecs/components/TimelineComponent.h"
#include "logic/ecs/components/TransformComponent.h"

namespace MMM::Logic::System
{

void NoteTransformSystem::update(entt::registry&       registry,
                                 const entt::registry& timelineRegistry,
                                 double                currentTime,
                                 float /* judgmentLineY 弃用 */)
{
    // 从时间线 registry 获取当前生效的流速
    auto   tlView = timelineRegistry.view<const TimelineComponent>();
    double currentScrollSpeed = 500.0;
    double maxTlTime          = -1.0;

    for ( auto entity : tlView ) {
        auto& tl = tlView.get<const TimelineComponent>(entity);
        if ( tl.m_effect == ::MMM::TimingEffect::SCROLL &&
             tl.m_timestamp <= currentTime && tl.m_timestamp > maxTlTime ) {
            maxTlTime          = tl.m_timestamp;
            currentScrollSpeed = tl.m_value;
        }
    }

    // 遍历音符计算逻辑位置
    auto noteView = registry.view<TransformComponent, const NoteComponent>();
    for ( auto entity : noteView ) {
        auto&       transform = noteView.get<TransformComponent>(entity);
        const auto& note      = noteView.get<const NoteComponent>(entity);

        // 计算相对时间的逻辑距离偏移
        float relativeDistance = static_cast<float>(
            (note.m_timestamp - currentTime) * currentScrollSpeed);

        // 此时 Transform.y 不包含视口的判断线坐标，纯粹是距离
        transform.m_pos.y = relativeDistance;
    }
}

void NoteRenderSystem::generateSnapshot(entt::registry& registry,
                                        RenderSnapshot* snapshot,
                                        float           viewportHeight,
                                        float           judgmentLineY)
{
    auto noteView =
        registry.view<const TransformComponent, const NoteComponent>();

    UI::BrushDrawCmd cmd;
    cmd.indexOffset  = 0;
    cmd.vertexOffset = 0;
    cmd.indexCount   = 0;
    cmd.texture      = VK_NULL_HANDLE;

    for ( auto entity : noteView ) {
        const auto& transform = noteView.get<const TransformComponent>(entity);
        const InteractionComponent* interaction =
            registry.try_get<InteractionComponent>(entity);
        bool isHovered = interaction ? interaction->isHovered : false;

        // 根据摄像机参数将逻辑 Y 转换为屏幕 Y
        float screenY = judgmentLineY - transform.m_pos.y;

        // 视口剔除（简单上下边界判断，考虑音符自身高度）
        if ( screenY + transform.m_size.y < 0.0f || screenY > viewportHeight ) {
            continue;
        }

        // 添加拾取包围盒
        snapshot->hitboxes.push_back({ entity,
                                       transform.m_pos.x,
                                       screenY,
                                       transform.m_size.x,
                                       transform.m_size.y });

        // 生成顶点 (矩形)
        uint32_t baseIndex = snapshot->vertices.size();

        // 左上，右上，右下，左下
        Graphic::Vertex::VKBasicVertex v1, v2, v3, v4;
        v1.pos = { transform.m_pos.x, screenY, 0.0f };
        v2.pos = { transform.m_pos.x + transform.m_size.x, screenY, 0.0f };
        v3.pos = { transform.m_pos.x + transform.m_size.x,
                   screenY + transform.m_size.y,
                   0.0f };
        v4.pos = { transform.m_pos.x, screenY + transform.m_size.y, 0.0f };

        // 悬停时高亮显示为橙色
        if ( isHovered ) {
            v1.color = v2.color = v3.color =
                v4.color        = { 1.0f, 0.5f, 0.0f, 1.0f };
        } else {
            v1.color = v2.color = v3.color =
                v4.color        = { 0.2f, 0.8f, 0.9f, 1.0f };
        }

        snapshot->vertices.push_back(v1);
        snapshot->vertices.push_back(v2);
        snapshot->vertices.push_back(v3);
        snapshot->vertices.push_back(v4);

        snapshot->indices.push_back(baseIndex + 0);
        snapshot->indices.push_back(baseIndex + 1);
        snapshot->indices.push_back(baseIndex + 2);
        snapshot->indices.push_back(baseIndex + 2);
        snapshot->indices.push_back(baseIndex + 3);
        snapshot->indices.push_back(baseIndex + 0);

        cmd.indexCount += 6;
    }

    if ( cmd.indexCount > 0 ) {
        snapshot->cmds.push_back(cmd);
    }
}

}  // namespace MMM::Logic::System

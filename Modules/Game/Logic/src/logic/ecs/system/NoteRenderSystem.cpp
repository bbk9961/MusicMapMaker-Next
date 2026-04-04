#include "logic/ecs/system/NoteRenderSystem.h"
#include "logic/ecs/components/InteractionComponent.h"
#include "logic/ecs/components/NoteComponent.h"
#include "logic/ecs/components/TransformComponent.h"
#include "logic/ecs/system/ScrollCache.h"

namespace MMM::Logic::System
{

void NoteRenderSystem::generateSnapshot(entt::registry&       registry,
                                        const entt::registry& timelineRegistry,
                                        RenderSnapshot*       snapshot,
                                        double                currentTime,
                                        float                 viewportHeight,
                                        float                 judgmentLineY)
{
    auto noteView =
        registry.view<const TransformComponent, const NoteComponent>();

    auto* cache = timelineRegistry.ctx().find<ScrollCache>();
    if ( !cache ) return;
    double currentAbsY = cache->getAbsY(currentTime);

    // 内部批处理器，负责根据 TextureID 自动切分 DrawCall
    struct Batcher {
        RenderSnapshot*  snapshot;
        TextureID        currentTex = TextureID::None;
        UI::BrushDrawCmd currentCmd;

        Batcher(RenderSnapshot* s) : snapshot(s)
        {
            currentCmd.indexOffset     = 0;
            currentCmd.vertexOffset    = 0;
            currentCmd.indexCount      = 0;
            currentCmd.texture         = VK_NULL_HANDLE;
            currentCmd.customTextureId = 0;
        }

        void setTexture(TextureID tex)
        {
            if ( currentCmd.indexCount > 0 && currentTex != tex ) {
                snapshot->cmds.push_back(currentCmd);
                currentCmd.indexCount   = 0;
                currentCmd.indexOffset  = snapshot->indices.size();
                currentCmd.vertexOffset = 0;
            }
            if ( currentCmd.indexCount == 0 ) {
                currentCmd.customTextureId = static_cast<uint32_t>(tex);
            }
            currentTex = tex;
        }

        /// @brief 推送一个矩形，如果是带纹理的，颜色固定为白色
        void pushQuad(float x, float y, float w, float h, glm::vec4 color)
        {
            uint32_t baseIndex = snapshot->vertices.size();

            Graphic::Vertex::VKBasicVertex v1, v2, v3, v4;
            v1.pos = { x, y, 0.0f };
            v2.pos = { x + w, y, 0.0f };
            v3.pos = { x + w, y - h, 0.0f };
            v4.pos = { x, y - h, 0.0f };

            // 标准 UV
            v1.uv = { 0.0f, 1.0f };
            v2.uv = { 1.0f, 1.0f };
            v3.uv = { 1.0f, 0.0f };
            v4.uv = { 0.0f, 0.0f };

            v1.color = v2.color = v3.color =
                v4.color        = { color.r, color.g, color.b, color.a };

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

            currentCmd.indexCount += 6;
        }

        void flush()
        {
            if ( currentCmd.indexCount > 0 ) {
                snapshot->cmds.push_back(currentCmd);
                currentCmd.indexCount = 0;
            }
        }
    } batcher(snapshot);

    for ( auto entity : noteView ) {
        const auto& transform = noteView.get<const TransformComponent>(entity);
        const auto& note      = noteView.get<const NoteComponent>(entity);
        const InteractionComponent* interaction =
            registry.try_get<InteractionComponent>(entity);
        bool isHovered = interaction ? interaction->isHovered : false;

        float screenY      = judgmentLineY - transform.m_pos.y;
        float visualHeight = transform.m_size.y;
        float x            = transform.m_pos.x;
        float w            = transform.m_size.x;

        // 简易视口剔除
        if ( screenY - visualHeight > viewportHeight ) continue;
        if ( screenY < 0.0f ) continue;

        // 同步拾取包围盒
        snapshot->hitboxes.push_back(
            { entity, x, screenY - visualHeight, w, visualHeight });

        // 设置颜色：如果是带贴图的，使用白色以保持贴图原色
        // 如果被悬停，则叠加一层橙色高亮 (0.5 混合)
        glm::vec4 texColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        if ( isHovered ) texColor = { 1.0f, 0.5f, 0.0f, 1.0f };

        if ( note.m_type == ::MMM::NoteType::NOTE ) {
            batcher.setTexture(TextureID::Note);
            batcher.pushQuad(x, screenY, w, 20.0f, texColor);
        } else if ( note.m_type == ::MMM::NoteType::HOLD ) {
            // Body
            batcher.setTexture(TextureID::HoldBodyVertical);
            batcher.pushQuad(x,
                             screenY - 10.0f,
                             w,
                             std::max(0.0f, visualHeight - 20.0f),
                             texColor);
            // End
            batcher.setTexture(TextureID::HoldEnd);
            batcher.pushQuad(
                x, screenY - visualHeight + 20.0f, w, 20.0f, texColor);
            // Head
            batcher.setTexture(TextureID::HoldHead);
            batcher.pushQuad(x, screenY, w, 20.0f, texColor);
        } else if ( note.m_type == ::MMM::NoteType::FLICK ) {
            batcher.setTexture(TextureID::HoldHead);
            batcher.pushQuad(x, screenY, w, 20.0f, texColor);

            if ( note.m_dtrack < 0 ) {
                batcher.setTexture(TextureID::FlickArrowLeft);
                batcher.pushQuad(x, screenY, w, 20.0f, texColor);
            } else if ( note.m_dtrack > 0 ) {
                batcher.setTexture(TextureID::FlickArrowRight);
                batcher.pushQuad(x, screenY, w, 20.0f, texColor);
            }
        } else if ( note.m_type == ::MMM::NoteType::POLYLINE ) {
            // 针对 Polyline 的多段绘制逻辑 (目前简化为子物件遍历)
            for ( size_t i = 0; i < note.m_subNotes.size(); ++i ) {
                const auto& sub     = note.m_subNotes[i];
                float       subX    = sub.trackIndex * 60.0f + 20.0f;
                double      subAbsY = cache->getAbsY(sub.timestamp);
                float       subScreenY =
                    judgmentLineY - static_cast<float>(subAbsY - currentAbsY);

                if ( sub.type == ::MMM::NoteType::HOLD ) {
                    double endAbsY =
                        cache->getAbsY(sub.timestamp + sub.duration);
                    float h = static_cast<float>(endAbsY - subAbsY);

                    batcher.setTexture(TextureID::HoldBodyVertical);
                    batcher.pushQuad(subX,
                                     subScreenY - 10.0f,
                                     w,
                                     std::max(0.0f, h - 20.0f),
                                     texColor);

                    if ( i == note.m_subNotes.size() - 1 ) {
                        batcher.setTexture(TextureID::HoldEnd);
                        batcher.pushQuad(
                            subX, subScreenY - h + 20.0f, w, 20.0f, texColor);
                    }
                } else if ( sub.type == ::MMM::NoteType::FLICK ) {
                    float nextX = (sub.trackIndex + sub.dtrack) * 60.0f + 20.0f;
                    float startX = std::min(subX, nextX);
                    float hw     = std::abs(nextX - subX);

                    batcher.setTexture(TextureID::HoldBodyHorizontal);
                    batcher.pushQuad(
                        startX, subScreenY, hw + w, 20.0f, texColor);

                    if ( sub.dtrack < 0 ) {
                        batcher.setTexture(TextureID::FlickArrowLeft);
                        batcher.pushQuad(nextX, subScreenY, w, 20.0f, texColor);
                    } else if ( sub.dtrack > 0 ) {
                        batcher.setTexture(TextureID::FlickArrowRight);
                        batcher.pushQuad(nextX, subScreenY, w, 20.0f, texColor);
                    }
                }

                if ( i == 0 ) {
                    batcher.setTexture(TextureID::HoldHead);
                    batcher.pushQuad(subX, subScreenY, w, 20.0f, texColor);
                }
            }
        }
    }

    batcher.flush();
}

}  // namespace MMM::Logic::System

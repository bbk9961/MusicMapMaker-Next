#include "logic/ecs/components/TimelineComponent.h"
#include "logic/ecs/system/NoteRenderSystem.h"
#include "logic/ecs/system/ScrollCache.h"
#include "logic/ecs/system/render/Batcher.h"
#include <algorithm>

namespace MMM::Logic::System
{

void NoteRenderSystem::renderTrackLayout(
    Batcher& batcher, float viewportWidth, float viewportHeight,
    float judgmentLineY, int32_t trackCount, const Config::EditorConfig& config,
    const entt::registry& timelineRegistry, double currentTime,
    const ScrollCache* cache, float& leftX, float& rightX, float& topY,
    float& bottomY, float& trackAreaW, float& singleTrackW)
{
    leftX            = viewportWidth * config.visual.trackLayout.left;
    rightX           = viewportWidth * config.visual.trackLayout.right;
    topY             = viewportHeight * config.visual.trackLayout.top;
    bottomY          = viewportHeight * config.visual.trackLayout.bottom;
    trackAreaW       = rightX - leftX;
    float trackAreaH = bottomY - topY;
    singleTrackW     = trackAreaW / static_cast<float>(trackCount);

    // 绘制轨道底板
    batcher.setTexture(TextureID::Track);
    auto uvIt =
        batcher.snapshot->uvMap.find(static_cast<uint32_t>(TextureID::Track));
    if ( uvIt != batcher.snapshot->uvMap.end() ) {
        // uvMap.z, w 是归一化的宽高，基于 2048 尺寸
        float texW_px = uvIt->second.z * 2048.0f;
        float texH_px = uvIt->second.w * 2048.0f;

        if ( texW_px > 0 && texH_px > 0 ) {
            float texAspect = texW_px / texH_px;
            float drawH =
                singleTrackW /
                texAspect;  // 每个轨道按照宽度等比缩放计算出的单块高度

            // 为了防止图集边缘线性采样溢出产生黑边，向内收缩 0.5 个像素的 UV
            const float halfPixelU = 0.5f / 2048.0f;
            const float halfPixelV = 0.5f / 2048.0f;

            float uMin = uvIt->second.x + halfPixelU;
            float uMax = uvIt->second.x + uvIt->second.z - halfPixelU;

            for ( int i = 0; i < trackCount; ++i ) {
                float trackX   = leftX + i * singleTrackW;
                float currentY = bottomY;  // 从底向上绘制

                // 稍微扩展一点宽度，防止浮点数精度导致的轨道间隙
                float drawW = singleTrackW + 0.5f;

                while ( currentY > topY ) {
                    float remainH     = currentY - topY;
                    float actualDrawH = std::min(drawH, remainH);

                    float vMax = 1.0f;
                    float vMin = 1.0f - (actualDrawH / drawH);

                    float finalVMin =
                        uvIt->second.y + vMin * uvIt->second.w + halfPixelV;
                    float finalVMax =
                        uvIt->second.y + vMax * uvIt->second.w - halfPixelV;

                    batcher.pushUVQuad(
                        trackX,
                        currentY,
                        drawW,
                        actualDrawH + 0.5f,  // 高度也稍微向下扩展防止横向接缝
                        glm::vec2(uMin, finalVMin),
                        glm::vec2(uMax, finalVMax),
                        { 1.0f, 1.0f, 1.0f, 1.0f });

                    currentY -= drawH;
                }
            }
        }
    }

    // 绘制轨道布局包围框
    batcher.setTexture(TextureID::None);
    batcher.pushStrokeRect(leftX,
                           topY,
                           rightX,
                           bottomY,
                           config.visual.trackBoxLineWidth,
                           { 0.5f, 0.5f, 0.5f, 1.0f });

    // 绘制判定区域 (按轨道分别绘制)
    batcher.setTexture(TextureID::JudgeArea);
    auto judgeUvIt = batcher.snapshot->uvMap.find(
        static_cast<uint32_t>(TextureID::JudgeArea));
    if ( judgeUvIt != batcher.snapshot->uvMap.end() ) {
        float texW = judgeUvIt->second.z * 2048.0f;
        float texH = judgeUvIt->second.w * 2048.0f;
        if ( texW > 0 && texH > 0 ) {
            float aspect = texW / texH;

            // 应用 noteScaleX 和 noteScaleY 缩放判定区
            float drawW = singleTrackW * config.visual.noteScaleX;
            float drawH = (singleTrackW / aspect) * config.visual.noteScaleY;

            // 为了防止图集边缘线性采样溢出产生黑边
            const float halfPixelU = 0.5f / 2048.0f;
            const float halfPixelV = 0.5f / 2048.0f;

            for ( int i = 0; i < trackCount; ++i ) {
                // 计算当前轨道的中心 X，然后根据缩放后的宽度求得实际绘制的 left
                // X
                float trackCenterX =
                    leftX + i * singleTrackW + singleTrackW * 0.5f;
                float drawX = trackCenterX - drawW * 0.5f;

                batcher.pushUVQuad(
                    drawX,
                    judgmentLineY + drawH * 0.5f,
                    drawW,
                    drawH,
                    glm::vec2(judgeUvIt->second.x + halfPixelU,
                              judgeUvIt->second.y + halfPixelV),
                    glm::vec2(
                        judgeUvIt->second.x + judgeUvIt->second.z - halfPixelU,
                        judgeUvIt->second.y + judgeUvIt->second.w - halfPixelV),
                    { 1.0f, 1.0f, 1.0f, 1.0f });
            }
        }
    } else {
        // Fallback: 绘制原有的纯色判定线
        batcher.setTexture(TextureID::None);
        batcher.pushQuad(leftX,
                         judgmentLineY + config.visual.judgelineWidth * 0.5f,
                         trackAreaW,
                         config.visual.judgelineWidth,
                         { 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // ================= 绘制拍线 =================
    if ( !cache ) return;

    int beatDivisor = config.settings.beatDivisor;
    if ( beatDivisor <= 0 ) beatDivisor = 4;

    // 筛选出所有 BPM 标记
    std::vector<const TimelineComponent*> bpmEvents;
    auto tlView = timelineRegistry.view<const TimelineComponent>();
    for ( auto entity : tlView ) {
        const auto& tl = tlView.get<const TimelineComponent>(entity);
        if ( tl.m_effect == ::MMM::TimingEffect::BPM ) {
            bpmEvents.push_back(&tl);
        }
    }

    if ( bpmEvents.empty() ) return;

    // 按时间升序
    std::stable_sort(
        bpmEvents.begin(),
        bpmEvents.end(),
        [](const TimelineComponent* a, const TimelineComponent* b) {
            return a->m_timestamp < b->m_timestamp;
        });

    double currentAbsY = cache->getAbsY(currentTime);
    double startTime =
        cache->getTime(currentAbsY - (viewportHeight - judgmentLineY));
    double endTime = cache->getTime(currentAbsY + judgmentLineY);

    batcher.setTexture(TextureID::None);
    // 假设轨道底板线宽 1.0，主拍头线设为 2.0，且更亮；分拍线设为 1.0 较暗
    glm::vec4 beatColor(1.0f, 1.0f, 1.0f, 0.4f);
    glm::vec4 subBeatColor(1.0f, 1.0f, 1.0f, 0.15f);

    for ( size_t i = 0; i < bpmEvents.size(); ++i ) {
        const auto* currentBPM = bpmEvents[i];
        double      bpmTime    = currentBPM->m_timestamp;
        double      bpmVal     = currentBPM->m_value;
        if ( bpmVal <= 0.0 ) bpmVal = 120.0;

        double nextBpmTime = (i + 1 < bpmEvents.size())
                                 ? bpmEvents[i + 1]->m_timestamp
                                 : std::numeric_limits<double>::infinity();

        if ( nextBpmTime <= startTime ) continue;
        if ( bpmTime >= endTime ) break;

        double beatDuration = 60.0 / bpmVal;
        double stepDuration = beatDuration / beatDivisor;

        // 找到第一个大于等于 startTime 且 >= bpmTime 的节拍时间点
        double startCalcTime = std::max(bpmTime, startTime);

        // 计算从 bpmTime 开始，到 startCalcTime 过了多少个 step
        // 加上一个小偏移量避免浮点误差
        int64_t stepOffset = 0;
        if ( startCalcTime > bpmTime ) {
            stepOffset = static_cast<int64_t>(
                std::ceil((startCalcTime - bpmTime) / stepDuration - 1e-4));
        }

        double t = bpmTime + stepOffset * stepDuration;

        while ( t < nextBpmTime && t <= endTime ) {
            // 当前 step 在这拍里的索引 (0 代表一拍的开头)
            int beatIndex = stepOffset % beatDivisor;

            bool isBeat = (beatIndex == 0);

            double absY = cache->getAbsY(t);
            float  y = judgmentLineY - static_cast<float>(absY - currentAbsY);

            // 只绘制屏幕范围内的线
            if ( y >= topY && y <= bottomY ) {
                if ( isBeat ) {
                    batcher.pushQuad(
                        leftX, y + 1.0f, trackAreaW, 2.0f, beatColor);
                } else {
                    batcher.pushQuad(
                        leftX, y + 0.5f, trackAreaW, 1.0f, subBeatColor);
                }
            }

            stepOffset++;
            t = bpmTime + stepOffset * stepDuration;
        }
    }
}

}  // namespace MMM::Logic::System

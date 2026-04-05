#include "logic/ecs/system/NoteRenderSystem.h"
#include "Batcher.h"
#include "config/skin/SkinConfig.h"
#include "logic/ecs/system/BackgroundRenderSystem.h"
#include "logic/ecs/system/ScrollCache.h"

namespace MMM::Logic::System
{

void NoteRenderSystem::generateSnapshot(
    entt::registry& registry, const entt::registry& timelineRegistry,
    RenderSnapshot* snapshot, const std::string& cameraId, double currentTime,
    float viewportWidth, float viewportHeight, float judgmentLineY,
    int32_t trackCount, const EditorConfig& config)
{
    const auto* cache = timelineRegistry.ctx().find<ScrollCache>();
    if ( !cache ) return;

    // 将 ScrollCache 临时注入 noteRegistry 以便在 renderNotes 中使用
    registry.ctx().insert_or_assign(cache);

    Batcher batcher(snapshot);
    float   leftX, rightX, topY, bottomY, trackAreaW, singleTrackW;
    float   renderScaleY = 1.0f;

    if ( cameraId == "Preview" ) {
        // 预览区逻辑：不使用主画布的轨道配置，使用预览区留白配置
        leftX        = config.previewConfig.margin.left;
        rightX       = viewportWidth - config.previewConfig.margin.right;
        topY         = config.previewConfig.margin.top;
        bottomY      = viewportHeight - config.previewConfig.margin.bottom;
        trackAreaW   = rightX - leftX;
        singleTrackW = trackAreaW / static_cast<float>(trackCount);
        renderScaleY = 1.0f / config.previewConfig.areaRatio;

        // 预览区通常不绘制背景图 (保持透明或由 UI 层处理)
        // 绘制主画布范围包围框和判定线
        auto& skin    = Config::SkinManager::instance();
        auto  boxCol  = skin.getColor("preview.boundingbox");
        auto  lineCol = skin.getColor("preview.judgeline");

        // 计算主画布在预览区中的可视范围
        // 我们假设主画布的高度与预览视口高度一致（作为参考基准）
        float mainVisibleH =
            (config.trackLayout.bottom - config.trackLayout.top) *
            viewportHeight * renderScaleY;
        // 主画布判定线在预览区中的位置（预览区 judgmentLineY 对应 currentTime）
        float mainJudgelineInPreviewY = judgmentLineY;

        // 包围框相对于判定线的位置
        float boxTop = mainJudgelineInPreviewY -
                       (config.judgeline_pos - config.trackLayout.top) *
                           viewportHeight * renderScaleY;

        batcher.setTexture(TextureID::None);
        // 绘制半透明背景包围框
        batcher.pushQuad(leftX,
                         boxTop + mainVisibleH,
                         trackAreaW,
                         mainVisibleH,
                         { boxCol.r, boxCol.g, boxCol.b, boxCol.a });
        // 绘制包围框轮廓 (不透明)
        batcher.pushStrokeRect(leftX,
                               boxTop,
                               rightX,
                               boxTop + mainVisibleH,
                               2.0f,
                               { boxCol.r, boxCol.g, boxCol.b, 1.0f });
        // 绘制判定线
        batcher.pushQuad(leftX,
                         mainJudgelineInPreviewY + config.judgelineWidth * 0.5f,
                         trackAreaW,
                         config.judgelineWidth,
                         { lineCol.r, lineCol.g, lineCol.b, lineCol.a });
    } else {
        // 主画布逻辑
        // 绘制背景
        BackgroundRenderSystem::render(
            batcher, viewportWidth, viewportHeight, config, snapshot);

        renderTrackLayout(batcher,
                          viewportWidth,
                          viewportHeight,
                          judgmentLineY,
                          trackCount,
                          config,
                          leftX,
                          rightX,
                          topY,
                          bottomY,
                          trackAreaW,
                          singleTrackW);
    }

    renderNotes(registry,
                snapshot,
                cameraId,
                currentTime,
                judgmentLineY,
                trackCount,
                config,
                batcher,
                leftX,
                rightX,
                topY,
                bottomY,
                singleTrackW,
                renderScaleY);

    batcher.flush();
}

}  // namespace MMM::Logic::System

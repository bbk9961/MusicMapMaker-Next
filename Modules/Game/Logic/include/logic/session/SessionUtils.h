#pragma once

#include "logic/session/context/SessionContext.h"
#include "logic/ecs/components/TimelineComponent.h"
#include <entt/entt.hpp>

namespace MMM::Logic::SessionUtils
{

struct SnapResult {
    bool   isSnapped{ false };
    double snappedTime{ 0.0 };
    int    numerator{ 0 };
    int    denominator{ 1 };
};

SnapResult getSnapResult(
    double rawTime, float mouseY, const CameraInfo& camera,
    const Config::EditorConfig& config,
    const std::vector<const TimelineComponent*>& bpmEvents,
    entt::registry& timelineRegistry,
    double visualTime,
    const std::unordered_map<std::string, CameraInfo>& cameras);

void syncHitIndex(SessionContext& ctx);
void rebuildHitEvents(SessionContext& ctx);

void loadBeatmap(SessionContext& ctx, std::shared_ptr<MMM::BeatMap> beatmap);
void syncBeatmap(SessionContext& ctx);

} // namespace MMM::Logic::SessionUtils

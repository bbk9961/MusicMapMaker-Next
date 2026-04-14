#pragma once

#include "config/EditorConfig.h"
#include "event/audio/AudioPlaybackEvent.h"
#include "logic/SyncClock.h"
#include "logic/ecs/components/NoteComponent.h"
#include "event/core/EventBus.h"
#include "logic/ecs/system/HitFXSystem.h"
#include "logic/session/EditorAction.h"
#include "mmm/beatmap/BeatMap.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace MMM::Logic
{

struct CameraInfo {
    std::string id;
    float       viewportWidth{ 800.0f };
    float       viewportHeight{ 600.0f };
};

struct MarqueeBox {
    double      startTime{ 0.0 };
    double      endTime{ 0.0 };
    float       startTrack{ 0.0f };
    float       endTrack{ 0.0f };
    std::string cameraId;
};

struct ClipboardItem {
    NoteComponent note;
};

/**
 * @brief 共享的上下文状态，供各个 Session Controller 和 Tool 访问。
 */
struct SessionContext {
    // --- 核心状态 ---
    entt::registry noteRegistry;
    entt::registry timelineRegistry;

    double currentTime{ 0.0 };
    double visualTime{ 0.0 };
    bool isPlaying{ false };
    int32_t trackCount{ 12 };

    std::shared_ptr<MMM::BeatMap> currentBeatmap;
    Config::EditorConfig lastConfig;
    std::unordered_map<std::string, CameraInfo> cameras;
    glm::vec2 bgSize{ 0.0f, 0.0f };

    // --- 音频与播放状态 ---
    double lastAudioPos{ 0.0 };
    double lastAudioSysTime{ 0.0 };
    SyncClock syncClock;
    double syncTimer{ 0.0 };

    std::vector<System::HitFXSystem::HitEvent> hitEvents;
    size_t nextHitIndex{ 0 };
    size_t nextPredictHitIndex{ 0 };
    System::HitFXSystem hitFXSystem;

    Event::ScopedSubscription<Event::AudioFinishedEvent> audioFinishedToken;
    Event::ScopedSubscription<Event::AudioPositionEvent> audioPositionToken;

    // --- 交互与工具状态 ---
    EditTool currentTool{ EditTool::Move };
    std::string  mouseCameraId;
    glm::vec2    lastMousePos{ 0.0f, 0.0f };
    entt::entity hoveredEntity{ entt::null };
    int32_t      hoveredPart{ 0 };
    int32_t      hoveredSubIndex{ -1 };
    bool         isMouseInCanvas{ false };
    bool         isDragging{ false };
    double       previewHoverTime{ 0.0 };
    double       previewEdgeScrollVelocity{ 0.0 };

    entt::entity draggedEntity{ entt::null };
    HoverPart draggedPart{ HoverPart::None };
    int draggedSubIndex{ -1 };
    std::optional<NoteComponent> dragInitialNote;
    std::string dragCameraId;

    bool                    isSelecting{ false };
    bool                    hasMarqueeSelection{ false };
    bool                    marqueeIsAdditive{ false };
    std::vector<MarqueeBox> marqueeBoxes;

    // --- 编辑操作栈 ---
    EditorActionStack actionStack;
    std::vector<ClipboardItem> clipboard;
};

} // namespace MMM::Logic

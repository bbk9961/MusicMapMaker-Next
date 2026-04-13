#include "config/AppConfig.h"
#include "event/core/EventBus.h"
#include "event/logic/LogicCommandEvent.h"
#include "log/colorful-log.h"
#include "logic/BeatmapSession.h"
#include "logic/ecs/components/InteractionComponent.h"
#include "logic/ecs/components/NoteComponent.h"
#include "logic/ecs/components/TimelineComponent.h"
#include "logic/ecs/components/TransformComponent.h"
#include "logic/ecs/system/render/Batcher.h"
#include "logic/session/NoteAction.h"
#include "mmm/beatmap/BeatMap.h"

namespace MMM::Logic
{

// --- Interaction Handlers ---

void BeatmapSession::handleCommand(const CmdSetHoveredEntity& cmd)
{
    if ( m_hoveredEntity != cmd.entity && m_hoveredEntity != entt::null ) {
        if ( m_noteRegistry.valid(m_hoveredEntity) &&
             m_noteRegistry.all_of<InteractionComponent>(m_hoveredEntity) ) {
            m_noteRegistry.get<InteractionComponent>(m_hoveredEntity)
                .isHovered = false;
            m_noteRegistry.get<InteractionComponent>(m_hoveredEntity)
                .hoveredPart = static_cast<uint8_t>(HoverPart::None);
        }
    }

    m_hoveredEntity   = cmd.entity;
    m_hoveredPart     = cmd.part;
    m_hoveredSubIndex = cmd.subIndex;

    if ( m_hoveredEntity != entt::null &&
         m_noteRegistry.valid(m_hoveredEntity) ) {
        if ( !m_noteRegistry.all_of<InteractionComponent>(m_hoveredEntity) ) {
            m_noteRegistry.emplace<InteractionComponent>(m_hoveredEntity);
        }
        auto& ic = m_noteRegistry.get<InteractionComponent>(m_hoveredEntity);
        ic.isHovered       = true;
        ic.hoveredPart     = cmd.part;
        ic.hoveredSubIndex = cmd.subIndex;
    }
}

void BeatmapSession::handleCommand(const CmdSelectEntity& cmd)
{
    // 只有在框选工具模式下才允许修改选中状态
    if ( m_currentTool != EditTool::Marquee ) return;

    // 如果清空其他选中，则也清空当前可能的框选留存
    if ( cmd.clearOthers ) {
        m_hasMarqueeSelection = false;
        auto view             = m_noteRegistry.view<InteractionComponent>();
        for ( auto entity : view ) {
            m_noteRegistry.get<InteractionComponent>(entity).isSelected = false;
        }
    }
    if ( cmd.entity != entt::null ) {
        if ( !m_noteRegistry.all_of<InteractionComponent>(cmd.entity) ) {
            m_noteRegistry.emplace<InteractionComponent>(cmd.entity);
        }
        auto& ic      = m_noteRegistry.get<InteractionComponent>(cmd.entity);
        ic.isSelected = !ic.isSelected;
    }
}

void BeatmapSession::handleCommand(const CmdStartDrag& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleStartDrag(*this, cmd);
    }
}

void BeatmapSession::handleCommand(const CmdUpdateDrag& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleUpdateDrag(*this, cmd);
    }
}

void BeatmapSession::handleCommand(const CmdEndDrag& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleEndDrag(*this, cmd);
    }
}

void BeatmapSession::handleCommand(const CmdSetMousePosition& cmd)
{
    bool canUpdate = false;
    if ( cmd.isHovering ) {
        if ( !m_isDragging || m_mouseCameraId == cmd.cameraId ||
             m_mouseCameraId == "" ) {
            m_mouseCameraId   = cmd.cameraId;
            m_isMouseInCanvas = true;
            canUpdate         = true;
        }
    } else if ( m_isDragging && m_mouseCameraId == cmd.cameraId ) {
        // 如果正在往外拖拽，依然允许更新坐标以便主画布跟随
        canUpdate = true;
    }

    if ( canUpdate ) {
        m_lastMousePos = { cmd.mouseX, cmd.mouseY };

        // 如果是从预览区发起的，或者正在预览区拖动，更新全局拖拽状态
        if ( cmd.cameraId == "Preview" ) {
            m_isDragging = cmd.isDragging;
        }

        // 预览区边缘滚动
        if ( cmd.cameraId == "Preview" && cmd.isDragging ) {
            auto it = m_cameras.find(cmd.cameraId);
            if ( it != m_cameras.end() ) {
                float margin = 20.0f;
                float dist   = 0.0f;
                if ( cmd.mouseY < margin )
                    dist = margin - cmd.mouseY;
                else if ( cmd.mouseY > it->second.viewportHeight - margin )
                    dist = (it->second.viewportHeight - margin) - cmd.mouseY;

                float sensitivity =
                    m_lastConfig.visual.previewConfig.edgeScrollSensitivity;
                m_previewEdgeScrollVelocity =
                    static_cast<double>(dist) * sensitivity;
            }
        } else if ( cmd.cameraId == "Preview" ) {
            m_previewEdgeScrollVelocity = 0.0;
        }
    } else {
        m_previewEdgeScrollVelocity = 0.0;
    }

    if ( !cmd.isHovering && !m_isDragging ) {
        if ( m_mouseCameraId == cmd.cameraId ) {
            m_mouseCameraId             = "";
            m_isMouseInCanvas           = false;
            m_previewEdgeScrollVelocity = 0.0;
        }
    }
}

void BeatmapSession::handleCommand(const CmdUpdateTrackCount& cmd)
{
    m_trackCount = cmd.trackCount;
    if ( m_currentBeatmap ) {
        m_currentBeatmap->m_baseMapMetadata.track_count = cmd.trackCount;
    }
}

void BeatmapSession::handleCommand(const CmdChangeTool& cmd)
{
    m_currentTool = cmd.tool;
}

void BeatmapSession::handleCommand(const CmdStartMarquee& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleStartMarquee(*this, cmd);
    }
}

void BeatmapSession::handleCommand(const CmdUpdateMarquee& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleUpdateMarquee(*this, cmd);
    }
}

void BeatmapSession::handleCommand(const CmdEndMarquee& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleEndMarquee(*this, cmd);
    }
}

void BeatmapSession::handleCommand(const CmdRemoveMarqueeAt& cmd)
{
    if ( m_tools.count(m_currentTool) ) {
        m_tools[m_currentTool]->handleRemoveMarqueeAt(*this, cmd);
    }
}

}  // namespace MMM::Logic

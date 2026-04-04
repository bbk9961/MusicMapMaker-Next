#include "logic/ecs/system/NoteTransformSystem.h"
#include "logic/ecs/components/NoteComponent.h"
#include "logic/ecs/components/TimelineComponent.h"
#include "logic/ecs/components/TransformComponent.h"
#include "logic/ecs/system/ScrollCache.h"

namespace MMM::Logic::System
{

void NoteTransformSystem::update(entt::registry& registry,
                                 entt::registry& timelineRegistry,
                                 double currentTime, float /* judgmentLineY */)
{
    auto& cache = timelineRegistry.ctx().get<ScrollCache>();
    if ( cache.isDirty ) {
        cache.rebuild(timelineRegistry);
    }

    double currentAbsY = cache.getAbsY(currentTime);

    auto noteView = registry.view<TransformComponent, const NoteComponent>();
    for ( auto entity : noteView ) {
        auto&       transform = noteView.get<TransformComponent>(entity);
        const auto& note      = noteView.get<const NoteComponent>(entity);

        double noteAbsY   = cache.getAbsY(note.m_timestamp);
        transform.m_pos.y = static_cast<float>(noteAbsY - currentAbsY);

        if ( note.m_type == ::MMM::NoteType::HOLD ) {
            double endAbsY = cache.getAbsY(note.m_timestamp + note.m_duration);
            transform.m_size.y = static_cast<float>(endAbsY - noteAbsY);
        } else if ( note.m_type == ::MMM::NoteType::POLYLINE &&
                    !note.m_subNotes.empty() ) {
            auto&  lastSub     = note.m_subNotes.back();
            double endTime     = lastSub.timestamp + lastSub.duration;
            double endAbsY     = cache.getAbsY(endTime);
            transform.m_size.y = static_cast<float>(endAbsY - noteAbsY);
        } else {
            transform.m_size.y = 20.0f;
        }
    }
}

}  // namespace MMM::Logic::System

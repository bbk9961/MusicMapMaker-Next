#pragma once

#include "common/LogicCommands.h"

namespace MMM::Logic
{
struct SessionContext;

/**
 * @brief Base interface for editor tools.
 * Tools encapsulate behavior for dragging, selection, creation, and
 * modification of notes.
 */
class IEditTool
{
public:
    virtual ~IEditTool() = default;

    virtual void onSelected(SessionContext& ctx) {}
    virtual void onDeselected(SessionContext& ctx) {}

    virtual void handleStartDrag(SessionContext&     ctx,
                                 const CmdStartDrag& cmd) {}
    virtual void handleUpdateDrag(SessionContext&      ctx,
                                  const CmdUpdateDrag& cmd) {}
    virtual void handleEndDrag(SessionContext& ctx, const CmdEndDrag& cmd) {}

    virtual void handleStartMarquee(SessionContext&        ctx,
                                    const CmdStartMarquee& cmd) {}
    virtual void handleUpdateMarquee(SessionContext&         ctx,
                                     const CmdUpdateMarquee& cmd) {}
    virtual void handleEndMarquee(SessionContext&      ctx,
                                  const CmdEndMarquee& cmd) {}
    virtual void handleRemoveMarqueeAt(SessionContext&           ctx,
                                       const CmdRemoveMarqueeAt& cmd) {}
};

}  // namespace MMM::Logic

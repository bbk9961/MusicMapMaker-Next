#pragma once

#include "logic/session/tool/IEditTool.h"

namespace MMM::Logic
{

class MarqueeTool : public IEditTool
{
public:
    void handleStartMarquee(SessionContext& ctx,
                            const CmdStartMarquee& cmd) override;
    void handleUpdateMarquee(SessionContext& ctx,
                             const CmdUpdateMarquee& cmd) override;
    void handleEndMarquee(SessionContext& ctx,
                          const CmdEndMarquee& cmd) override;
    void handleRemoveMarqueeAt(SessionContext& ctx,
                               const CmdRemoveMarqueeAt& cmd) override;
};

}  // namespace MMM::Logic

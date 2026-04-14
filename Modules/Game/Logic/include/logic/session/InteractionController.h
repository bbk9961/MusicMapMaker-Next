#pragma once

#include "common/LogicCommands.h"
#include "logic/session/tool/IEditTool.h"

namespace MMM::Logic
{
struct SessionContext;

class InteractionController
{
public:
    InteractionController(SessionContext& ctx);

    void handleCommand(const CmdSetHoveredEntity& cmd);
    void handleCommand(const CmdSelectEntity& cmd);
    void handleCommand(const CmdStartDrag& cmd);
    void handleCommand(const CmdUpdateDrag& cmd);
    void handleCommand(const CmdEndDrag& cmd);
    void handleCommand(const CmdChangeTool& cmd);
    void handleCommand(const CmdSetMousePosition& cmd);
    void handleCommand(const CmdStartMarquee& cmd);
    void handleCommand(const CmdUpdateMarquee& cmd);
    void handleCommand(const CmdEndMarquee& cmd);
    void handleCommand(const CmdRemoveMarqueeAt& cmd);
    void handleCommand(const CmdUpdateTrackCount& cmd);

    void updateMarqueeSelection(bool forceFullSync = false);

private:
    SessionContext&                                          m_ctx;
    std::unordered_map<EditTool, std::unique_ptr<IEditTool>> m_tools;
};

}  // namespace MMM::Logic

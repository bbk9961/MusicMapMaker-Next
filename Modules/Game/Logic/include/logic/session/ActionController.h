#pragma once

#include "common/LogicCommands.h"

namespace MMM::Logic
{
struct SessionContext;

class ActionController
{
public:
    ActionController(SessionContext& ctx) : m_ctx(ctx) {}

    void handleCommand(const CmdUndo& cmd);
    void handleCommand(const CmdRedo& cmd);
    void handleCommand(const CmdCopy& cmd);
    void handleCommand(const CmdCut& cmd);
    void handleCommand(const CmdPaste& cmd);
    void handleCommand(const CmdUpdateTimelineEvent& cmd);
    void handleCommand(const CmdDeleteTimelineEvent& cmd);
    void handleCommand(const CmdCreateTimelineEvent& cmd);

    void syncBeatmap();

private:
    SessionContext& m_ctx;
};

} // namespace MMM::Logic

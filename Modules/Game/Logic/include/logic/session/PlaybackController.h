#pragma once

#include "common/LogicCommands.h"

namespace MMM::Logic
{
struct SessionContext;

class PlaybackController
{
public:
    PlaybackController(SessionContext& ctx) : m_ctx(ctx) {}

    void handleCommand(const CmdSetPlayState& cmd);
    void handleCommand(const CmdSeek& cmd);
    void handleCommand(const CmdSetPlaybackSpeed& cmd);
    void handleCommand(const CmdScroll& cmd);

    void syncHitIndex();
    void rebuildHitEvents();
    void onUpdate(double dt);

private:
    SessionContext& m_ctx;
};

} // namespace MMM::Logic

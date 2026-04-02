#pragma once

#include "mmm/note/Hold.h"
#include "mmm/note/Note.h"
#include "mmm/timing/Timing.h"
#include <functional>
#include <vector>

namespace MMM
{

class BeatMap
{
public:
    BeatMap();
    BeatMap(BeatMap&&)                 = default;
    BeatMap(const BeatMap&)            = default;
    BeatMap& operator=(BeatMap&&)      = default;
    BeatMap& operator=(const BeatMap&) = default;
    ~BeatMap();

    /// @brief 所有物件引用
    std::vector<std::reference_wrapper<Note>> m_allNotes;

    /// @brief 所有普通物件
    std::vector<Note> m_notes;

    /// @brief 所有长条物件
    std::vector<Hold> m_holds;

    /// @brief 所有时间线
    std::vector<Timing> m_timings;
};

}  // namespace MMM

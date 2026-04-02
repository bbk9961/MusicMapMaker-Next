#pragma once

#include "mmm/note/Note.h"

namespace MMM
{

class Hold : public Note
{
public:
    Hold()                       = default;
    Hold(Hold&&)                 = default;
    Hold(const Hold&)            = default;
    Hold& operator=(Hold&&)      = default;
    Hold& operator=(const Hold&) = default;
    ~Hold()                      = default;

    /// @brief 长条持续时间
    double m_duration{ .0 };
};

}  // namespace MMM

#pragma once

namespace MMM
{

enum class NoteType {
    NOTE,
    HOLD,
};

class Note
{
public:
    Note();
    Note(Note&&)                 = default;
    Note(const Note&)            = default;
    Note& operator=(Note&&)      = default;
    Note& operator=(const Note&) = default;
    ~Note();

    /// @brief 物件类型
    NoteType m_type{ NoteType::NOTE };

    /// @brief 物件时间戳
    double m_timestamp{ 0 };
};


}  // namespace MMM

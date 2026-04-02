#pragma once

namespace MMM
{
enum class TimingEffect {
    // bpm效果
    BPM,
    // scroll效果
    SCROLL,
};

class Timing
{
public:
    Timing()                         = default;
    Timing(Timing&&)                 = default;
    Timing(const Timing&)            = default;
    Timing& operator=(Timing&&)      = default;
    Timing& operator=(const Timing&) = default;
    ~Timing()                        = default;

    /// @brief 时间线所处时间戳
    double m_timestamp{ 0 };

    /// @brief 时间线效果类型
    TimingEffect m_timingEffect{ TimingEffect::BPM };

    /// @brief 时间线效果参数
    double m_timingEffectParameter;
};


}  // namespace MMM

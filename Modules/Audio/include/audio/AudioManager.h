#pragma once

#include <memory>
#include <string>

namespace ice
{
class AudioPool;
class SDLPlayer;
class SourceNode;
class ThreadPool;
class MixBus;
class TimeStretcher;
}  // namespace ice

namespace MMM::Audio
{

/**
 * @brief 播放状态
 */
enum class PlaybackStatus { Stopped, Playing, Paused };

/**
 * @brief 音频管理器，封装 IonCachyEngine 的核心功能
 */
class AudioManager
{
public:
    static AudioManager& instance();

    /// @brief 初始化音频后端和引擎
    void init();

    /// @brief 关闭音频引擎
    void shutdown();

    /// @brief 加载 BGM
    /// @param filePath 音频文件绝对路径
    /// @return 是否加载成功
    bool loadBGM(const std::string& filePath);

    /// @brief 开始/恢复播放
    void play();

    /// @brief 暂停播放
    void pause();

    /// @brief 停止播放并回到起始位置
    void stop();

    /// @brief 跳转到指定时间
    /// @param seconds 秒
    void seek(double seconds);

    /// @brief 获取当前播放状态
    PlaybackStatus getStatus() const;

    /// @brief 获取当前播放时间 (秒)
    double getCurrentTime() const;

    /// @brief 获取总时长 (秒)
    double getTotalTime() const;

    /// @brief 设置音量 (0.0 ~ 1.0)
    void setVolume(float volume);

    /// @brief 获取当前音量
    float getVolume() const;

    /// @brief 设置播放倍率 (0.5 ~ 2.0)
    void setPlaybackSpeed(double speed);

    /// @brief 获取当前播放倍率
    double getPlaybackSpeed() const;

private:
    AudioManager()  = default;
    ~AudioManager() = default;

    std::unique_ptr<ice::ThreadPool> m_threadPool;
    std::unique_ptr<ice::AudioPool>  m_audioPool;
    std::unique_ptr<ice::SDLPlayer>  m_player;

    std::shared_ptr<ice::SourceNode>    m_bgmSource;
    std::shared_ptr<ice::TimeStretcher> m_stretcher;
    std::shared_ptr<ice::MixBus>        m_mixer;

    PlaybackStatus m_status{ PlaybackStatus::Stopped };
    float          m_volume{ 0.5f };
    double         m_speed{ 1.0 };
};

}  // namespace MMM::Audio

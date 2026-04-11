#include "audio/AudioManager.h"
#include "audio/SoundEffectPool.h"
#include "config/AppConfig.h"
#include "event/audio/AudioPlaybackEvent.h"
#include "event/core/EventBus.h"
#include "log/colorful-log.h"
#include "mmm/project/AudioResource.h"

#include <ice/core/MixBus.hpp>
#include <ice/core/PlayCallBack.hpp>
#include <ice/core/SourceNode.hpp>
#include <ice/core/effect/TimeStretcher.hpp>
#include <ice/manage/AudioPool.hpp>
#include <ice/out/play/sdl/SDLPlayer.hpp>
#include <ice/thread/ThreadPool.hpp>

namespace MMM::Audio
{

class AudioPlayCallBack : public ice::PlayCallBack
{
public:
    void play_done(bool loop) const override
    {
        if ( !loop ) {
            Event::AudioFinishedEvent e;
            e.isLooping = loop;
            Event::EventBus::instance().publish(e);
        }
    }

    void frameplaypos_updated(size_t frame_pos) override {}

    void timeplaypos_updated(std::chrono::nanoseconds time_pos) override
    {
        Event::AudioPositionEvent e;
        e.positionSeconds = std::chrono::duration<double>(time_pos).count();
        e.systemTimeSeconds =
            std::chrono::duration<double>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count();
        Event::EventBus::instance().publish(e);
    }
};

static std::shared_ptr<AudioPlayCallBack> g_callback =
    std::make_shared<AudioPlayCallBack>();

AudioManager& AudioManager::instance()
{
    static AudioManager inst;
    return inst;
}

AudioManager::AudioManager()
{
    // 从配置初始化音量
    auto& settings    = Config::AppConfig::instance().getEditorSettings();
    m_globalVolume    = settings.globalVolume;
    m_mainTrackVolume = 0.5f;  // 默认主音轨音量
}


AudioManager::~AudioManager() = default;

void AudioManager::init()
{
    XINFO("Initializing AudioManager...");
    ice::SDLPlayer::init_backend();

    m_threadPool = std::make_unique<ice::ThreadPool>(4);
    m_audioPool  = std::make_unique<ice::AudioPool>();
    m_player     = std::make_unique<ice::SDLPlayer>();

    m_mixer     = std::make_shared<ice::MixBus>();
    m_stretcher = std::make_shared<ice::TimeStretcher>();

    m_stretcher->set_inputnode(m_mixer);
    m_player->set_source(m_stretcher);

    if ( !m_player->open() ) {
        XERROR("Failed to open SDL audio device.");
    }
    m_player->start();
    XINFO("AudioManager initialized.");
}

void AudioManager::shutdown()
{
    XINFO("Shutting down AudioManager...");
    if ( m_player ) {
        m_player->stop();
        m_player->close();
    }
    ice::SDLPlayer::quit_backend();

    m_bgmSource.reset();
    m_stretcher.reset();
    m_mixer.reset();
    m_player.reset();
    m_audioPool.reset();
    m_threadPool.reset();
    XINFO("AudioManager shutdown.");
}

bool AudioManager::loadBGM(const std::string&      filePath,
                           const AudioTrackConfig& config)
{
    if ( !m_audioPool || !m_threadPool ) return false;

    XINFO("Loading BGM: {}", filePath);
    auto trackWeak = m_audioPool->get_or_load(*m_threadPool, filePath);
    auto track     = trackWeak.lock();

    if ( !track ) {
        XERROR("Failed to load audio track: {}", filePath);
        return false;
    }

    stop();

    if ( m_bgmSource ) {
        m_mixer->remove_source(m_bgmSource);
    }

    m_mainTrackVolume = config.volume;

    m_bgmSource = std::make_shared<ice::SourceNode>(track);
    m_bgmSource->setvolume(config.muted ? 0.0f
                                        : m_mainTrackVolume * m_globalVolume);
    m_bgmSource->add_playcallback(g_callback);

    m_mixer->add_source(m_bgmSource);

    // 应用播放速度
    setPlaybackSpeed(config.playbackSpeed);

    XINFO("BGM loaded successfully.");
    return true;
}

void AudioManager::play()
{
    if ( m_bgmSource ) {
        m_bgmSource->play();
        m_status = PlaybackStatus::Playing;
    }
}

void AudioManager::pause()
{
    if ( m_bgmSource ) {
        m_bgmSource->pause();
        m_status = PlaybackStatus::Paused;
    }
}

void AudioManager::stop()
{
    if ( m_bgmSource ) {
        m_bgmSource->pause();
        m_bgmSource->set_playpos(static_cast<size_t>(0));
        m_status = PlaybackStatus::Stopped;
    }
}

void AudioManager::seek(double seconds)
{
    if ( m_bgmSource ) {
        m_bgmSource->set_playpos(std::chrono::duration<double>(seconds));
    }
}

PlaybackStatus AudioManager::getStatus() const
{
    return m_status;
}

double AudioManager::getCurrentTime() const
{
    if ( !m_bgmSource ) return 0.0;

    auto   pos = m_bgmSource->get_playpos();
    double samplerate =
        static_cast<double>(ice::ICEConfig::internal_format.samplerate);
    if ( samplerate <= 0 ) return 0.0;

    return static_cast<double>(pos) / samplerate;
}

double AudioManager::getTotalTime() const
{
    if ( !m_bgmSource ) return 0.0;
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               m_bgmSource->total_time())
        .count();
}

void AudioManager::setMainTrackVolume(float volume)
{
    m_mainTrackVolume = std::clamp(volume, 0.0f, 1.0f);
    if ( m_bgmSource ) {
        m_bgmSource->setvolume(m_mainTrackVolume * m_globalVolume);
    }
}

float AudioManager::getMainTrackVolume() const
{
    return m_mainTrackVolume;
}

void AudioManager::setGlobalVolume(float volume)
{
    m_globalVolume = std::clamp(volume, 0.0f, 1.0f);

    // 同步到配置并保存
    auto& settings        = Config::AppConfig::instance().getEditorSettings();
    settings.globalVolume = m_globalVolume;
    Config::AppConfig::instance().save();

    // 重新应用全局音量到主音轨
    if ( m_bgmSource ) {
        m_bgmSource->setvolume(m_mainTrackVolume * m_globalVolume);
    }
}

float AudioManager::getGlobalVolume() const
{
    return m_globalVolume;
}

void AudioManager::setPlaybackSpeed(double speed)
{
    m_speed = std::clamp(speed, 0.1, 4.0);
    if ( m_stretcher ) {
        m_stretcher->set_playback_ratio(m_speed);
    }
}

double AudioManager::getPlaybackSpeed() const
{
    return m_speed;
}

void AudioManager::setSFXPoolVolume(const std::string& key, float volume,
                                    bool isPermanent)
{
    auto it = m_sfxPools.find(key);
    if ( it != m_sfxPools.end() ) {
        it->second->setVolume(volume);

        if ( isPermanent ) {
            // 保存到编辑器配置
            auto& sfxCfg =
                Config::AppConfig::instance().getEditorSettings().sfxConfig;
            sfxCfg.permanentSfxVolumes[key] = volume;
            Config::AppConfig::instance().save();
        }
    }
}

float AudioManager::getSFXPoolVolume(const std::string& key) const
{
    auto it = m_sfxPools.find(key);
    if ( it != m_sfxPools.end() ) {
        return it->second->getVolume();
    }
    return 1.0f;
}

double AudioManager::getSFXDuration(const std::string& key) const
{
    auto it = m_sfxPools.find(key);
    if ( it != m_sfxPools.end() ) {
        return it->second->getDuration();
    }
    return 0.0;
}

double AudioManager::getSFXPlaybackTime(const std::string& key) const
{
    auto it = m_sfxPools.find(key);
    if ( it != m_sfxPools.end() ) {
        return it->second->getLatestPlaybackTime();
    }
    return 0.0;
}

bool AudioManager::preloadSoundEffect(const std::string& key,
                                      const std::string& filePath,
                                      float              defaultVolume)
{
    if ( !m_audioPool || !m_threadPool || !m_mixer ) return false;

    // 检查是否已经有配置好的音量 (来自 EditorSettings 或之前的加载)
    float activeVolume = defaultVolume;
    auto& sfxCfg = Config::AppConfig::instance().getEditorSettings().sfxConfig;
    if ( sfxCfg.permanentSfxVolumes.count(key) ) {
        activeVolume = sfxCfg.permanentSfxVolumes.at(key);
    }

    XINFO(
        "Preloading SFX: {} from {} (Volume: {})", key, filePath, activeVolume);
    auto trackWeak = m_audioPool->get_or_load(*m_threadPool, filePath);
    auto track     = trackWeak.lock();

    if ( !track ) {
        XERROR("Failed to load SFX track: {}", filePath);
        return false;
    }

    auto pool = std::make_shared<SoundEffectPool>(track, m_mixer);
    pool->init(8);  // 预分配 8 个并发节点
    pool->setVolume(activeVolume);

    m_sfxPools[key] = std::move(pool);
    return true;
}

void AudioManager::playSoundEffect(const std::string& key, float volumeFactor)
{
    auto it = m_sfxPools.find(key);
    if ( it == m_sfxPools.end() ) return;

    it->second->play(m_globalVolume * it->second->getVolume() * volumeFactor);
}

void AudioManager::playSoundEffectScheduled(const std::string& key,
                                            double             targetTime,
                                            float              volumeFactor)
{
    auto it = m_sfxPools.find(key);
    if ( it == m_sfxPools.end() ) return;

    if ( !m_bgmSource ) return;

    double samplerate =
        static_cast<double>(ice::ICEConfig::internal_format.samplerate);
    size_t targetFrame = static_cast<size_t>(targetTime * samplerate);

    // 获取 BGM 播放位置的闭包，用于 SourceNode 内部参考
    auto bgmRef = [this]() -> size_t {
        if ( m_bgmSource ) return m_bgmSource->get_playpos();
        return 0;
    };

    it->second->playScheduled(
        m_globalVolume * it->second->getVolume() * volumeFactor,
        targetFrame,
        bgmRef);
}

}  // namespace MMM::Audio

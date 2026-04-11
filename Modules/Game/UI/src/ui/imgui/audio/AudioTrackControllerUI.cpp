#include "ui/imgui/audio/AudioTrackControllerUI.h"
#include "audio/AudioManager.h"
#include "config/skin/translation/Translation.h"
#include "imgui.h"
#include "logic/EditorEngine.h"
#include "ui/UIManager.h"

namespace MMM::UI
{

AudioTrackControllerUI::AudioTrackControllerUI(const std::string& trackId,
                                               const std::string& trackName,
                                               TrackType          type)
    : IUIView(trackName)
    , m_trackId(trackId)
    , m_trackName(trackName)
    , m_type(type)
{
}

void AudioTrackControllerUI::update(UIManager* sourceManager)
{
    if ( !m_isOpen ) {
        return;
    }

    auto& audio   = Audio::AudioManager::instance();
    auto& engine  = Logic::EditorEngine::instance();
    auto* project = engine.getCurrentProject();

    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);
    if ( ImGui::Begin(m_trackName.c_str(), &m_isOpen) ) {
        float volume = 0.0f;
        float speed  = 1.0f;
        bool  muted  = false;

        bool changed = false;

        // 获取当前状态
        if ( m_type == TrackType::Main ) {
            volume = audio.getMainTrackVolume();
            speed  = (float)audio.getPlaybackSpeed();
            // TODO: Main track mute state? Assume false for now as it's the
            // core
        } else {
            volume = audio.getSFXPoolVolume(m_trackId);
        }

        // 1. 音量控制
        if ( ImGui::SliderFloat(TR("ui.audio_manager.volume").data(),
                                &volume,
                                0.0f,
                                1.0f,
                                "%.2f") ) {
            changed = true;
            if ( m_type == TrackType::Main ) {
                audio.setMainTrackVolume(volume);
            } else {
                // 判断是否是常驻音效（皮肤）还是项目音效
                bool  isPermanent = true;
                auto& skinData    = Config::SkinManager::instance().getData();
                if ( skinData.audioPaths.count(m_trackId) == 0 ) {
                    isPermanent = false;
                }
                audio.setSFXPoolVolume(m_trackId, volume, isPermanent);
            }
        }

        // 2. 播放速度控制 (仅对 Main 有效，SFX 目前不支持变速播放)
        if ( m_type == TrackType::Main ) {
            if ( ImGui::SliderFloat("Speed", &speed, 0.5f, 2.0f, "%.2fx") ) {
                changed = true;
                audio.setPlaybackSpeed(speed);
            }
        }

        // 3. 特效音轨预览控件
        if ( m_type == TrackType::Effect ) {
            ImGui::Separator();
            if ( ImGui::Button(TR("ui.audio_manager.play_preview").data()) ) {
                audio.playSoundEffect(m_trackId);
            }

            ImGui::SameLine();
            float duration     = (float)audio.getSFXDuration(m_trackId);
            float playbackTime = (float)audio.getSFXPlaybackTime(m_trackId);
            float progress =
                (duration > 0.0f) ? (playbackTime / duration) : 0.0f;
            std::string progressText =
                fmt::format("{:.2f}s / {:.2f}s", playbackTime, duration);

            ImGui::ProgressBar(progress, ImVec2(-1, 0), progressText.c_str());
        }

        // 4. 应用更改并持久化
        if ( changed ) {
            if ( m_type == TrackType::Main ) {
                // 同步到项目配置
                if ( project ) {
                    for ( auto& res : project->m_audioResources ) {
                        if ( res.m_id == m_trackId ) {
                            res.m_config.volume        = volume;
                            res.m_config.playbackSpeed = speed;
                        }
                    }
                    engine.saveProject();
                }
            } else {
                if ( project ) {
                    for ( auto& res : project->m_audioResources ) {
                        if ( res.m_id == m_trackId ) {
                            res.m_config.volume = volume;
                        }
                    }
                    engine.saveProject();
                }
            }
        }
    }
    ImGui::End();
}

}  // namespace MMM::UI

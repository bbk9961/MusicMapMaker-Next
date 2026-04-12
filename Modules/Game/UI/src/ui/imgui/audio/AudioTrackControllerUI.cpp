#include "ui/imgui/audio/AudioTrackControllerUI.h"
#include "audio/AudioManager.h"
#include "config/skin/translation/Translation.h"
#include "imgui.h"
#include "logic/EditorEngine.h"
#include "ui/Icons.h"
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

    ImGui::SetNextWindowSize(ImVec2(350, 150), ImGuiCond_FirstUseEver);
    if ( ImGui::Begin(m_trackName.c_str(), &m_isOpen) ) {
        float volume = 0.5f;
        float speed  = 1.0f;
        bool  muted  = false;

        AudioTrackConfig* config = nullptr;
        if ( project ) {
            for ( auto& res : project->m_audioResources ) {
                if ( res.m_id == m_trackId ) {
                    config = &res.m_config;
                    break;
                }
            }
        }

        if ( config ) {
            volume = config->volume;
            speed  = config->playbackSpeed;
            muted  = config->muted;
        } else {
            // 回退到内存状态
            if ( m_type == TrackType::Main ) {
                volume = audio.getMainTrackVolume();
                speed  = (float)audio.getPlaybackSpeed();
                muted  = audio.isMainTrackMuted();
            } else {
                volume = audio.getSFXPoolVolume(m_trackId);
                muted  = audio.getSFXPoolMute(m_trackId);
            }
        }

        bool changed = false;

        // --- 1. 静音按钮与音量图标 ---
        const char* icon = ICON_MMM_VOLUME_MUTE;
        if ( !muted ) {
            if ( volume <= 0.33f )
                icon = ICON_MMM_VOLUME_OFF;
            else if ( volume <= 0.66f )
                icon = ICON_MMM_VOLUME_LOW;
            else
                icon = ICON_MMM_VOLUME_HIGH;
        }

        bool pushedTextColor = false;
        if ( muted ) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            pushedTextColor = true;
        }
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        if ( ImGui::Button(icon, ImVec2(30, 0)) ) {
            muted   = !muted;
            changed = true;
        }
        ImGui::PopStyleColor();  // Pop ImGuiCol_Button
        if ( pushedTextColor ) {
            ImGui::PopStyleColor();  // Pop ImGuiCol_Text
        }
        if ( ImGui::IsItemHovered() ) {
            ImGui::SetTooltip(muted ? "Unmute" : "Mute");
        }

        ImGui::SameLine();

        // --- 2. 音量拉条 ---
        ImGui::SetNextItemWidth(-50);
        if ( ImGui::SliderFloat("##Volume", &volume, 0.0f, 1.0f, "%.2f") ) {
            changed = true;
            // 如果拉动了进度条，自动解除静音（可选，但通常用户期望这样）
            if ( muted && volume > 0.0f ) {
                muted = false;
            }
        }

        // --- 3. 播放速度控制 (仅对 Main 有效) ---
        if ( m_type == TrackType::Main ) {
            if ( ImGui::SliderFloat("Speed", &speed, 0.5f, 2.0f, "%.2fx") ) {
                changed = true;
            }
        }

        // --- 4. 特效音轨预览控件 ---
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

        // --- 5. 应用更改并持久化 ---
        if ( changed ) {
            if ( config ) {
                config->volume        = volume;
                config->muted         = muted;
                config->playbackSpeed = speed;
            }

            if ( m_type == TrackType::Main ) {
                audio.setMainTrackVolume(volume);
                audio.setMainTrackMute(muted);
                audio.setPlaybackSpeed(speed);
            } else {
                bool  isPermanent = true;
                auto& skinData    = Config::SkinManager::instance().getData();
                if ( skinData.audioPaths.count(m_trackId) == 0 ) {
                    isPermanent = false;
                }
                audio.setSFXPoolVolume(m_trackId, volume, isPermanent);
                audio.setSFXPoolMute(m_trackId, muted, isPermanent);
            }

            if ( project ) {
                engine.saveProject();
            }
        }
    }
    ImGui::End();
}

}  // namespace MMM::UI

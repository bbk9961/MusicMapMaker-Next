#include "ui/imgui/manager/AudioManagerView.h"
#include "config/AppConfig.h"
#include "config/skin/SkinConfig.h"
#include "config/skin/translation/Translation.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "logic/EditorEngine.h"
#include "ui/layout/box/CLayBox.h"

namespace MMM::UI
{
// 内部绘制逻辑 (Clay/ImGui)
void AudioManagerView::onUpdate(LayoutContext& layoutContext,
                                UIManager*     sourceManager)
{
    auto& engine  = Logic::EditorEngine::instance();
    auto* project = engine.getCurrentProject();
    auto& skinCfg = Config::SkinManager::instance();

    ImFont* fileManagerFont = skinCfg.getFont("filemanager");
    if ( fileManagerFont ) ImGui::PushFont(fileManagerFont);

    CLayVBox rootVBox;

    if ( !project ) {
        CLayHBox labelHBox;
        auto     fh = ImGui::GetFrameHeight();
        labelHBox.addSpring()
            .addElement("InitialHint",
                        Sizing::Grow(),
                        Sizing::Fixed(fh),
                        [=](Clay_BoundingBox r, bool isHovered) {
                            float offY =
                                (r.height - ImGui::GetFontSize()) * 0.5f;
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offY);
                            ImVec2 textSize = ImGui::CalcTextSize(
                                TR("ui.audio_manager.initial_hint"));
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                                 (r.width - textSize.x) * 0.5f);
                            ImGui::TextEx(TR("ui.audio_manager.initial_hint"));
                        })
            .addSpring();

        rootVBox.setPadding(12, 12, 12, 12)
            .addLayout(
                "labelHBox", labelHBox, Sizing::Grow(), Sizing::Fixed(40));
        rootVBox.addSpring();
        rootVBox.render(layoutContext);

        if ( fileManagerFont ) ImGui::PopFont();
        return;
    }

    // 已打开项目时的界面
    CLayVBox listVBox;
    listVBox.setSpacing(4);

    listVBox.addElement(
        "AudioTracksHeader",
        Sizing::Grow(),
        Sizing::Fixed(24),
        [this](Clay_BoundingBox r, bool isHovered) {
            float indent = ImGui::CalcTextSize("AA").x;
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
            ImGui::SeparatorText(TR("ui.audio_manager.audio_tracks").data());
        });

    for ( const auto& audio : project->m_audioResources ) {
        listVBox.addElement("Audio_" + audio.m_id,
                            Sizing::Grow(),
                            Sizing::Fixed(28),
                            [&audio](Clay_BoundingBox r, bool isHovered) {
                                ImGui::Indent();
                                ImGui::Selectable(audio.m_id.c_str());
                                if ( ImGui::IsItemHovered() ) {
                                    ImGui::SetTooltip(
                                        "%s (Type: %s)",
                                        audio.m_path.c_str(),
                                        audio.m_type == AudioTrackType::Main
                                            ? "Main"
                                            : "Effect");
                                }
                                ImGui::Unindent();
                            });
    }

    rootVBox.setPadding(12, 12, 12, 12)
        .setSpacing(12)
        .addLayout("listVBox", listVBox, Sizing::Grow(), Sizing::Grow())
        .addSpring();
    rootVBox.render(layoutContext);

    ImGui::PopStyleVar();

    if ( fileManagerFont ) ImGui::PopFont();
}
}  // namespace MMM::UI

#include "ui/imgui/menu/MainMenuView.h"
#include "common/LogicCommands.h"
#include "config/AppConfig.h"
#include "config/skin/SkinConfig.h"
#include "event/core/EventBus.h"
#include "event/logic/LogicCommandEvent.h"
#include "event/ui/menu/OpenProjectEvent.h"
#include "log/colorful-log.h"
#include "ui/Icons.h"
#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>

namespace MMM::UI
{

MainMenuView::MainMenuView() {}

MainMenuView::~MainMenuView() {}

void MainMenuView::dispatchCommand(const MMM::Logic::LogicCommand& cmd)
{
    Event::EventBus::instance().publish(Event::LogicCommandEvent(cmd));
}

void MainMenuView::handleHotkeys()
{
    ImGuiIO& io = ImGui::GetIO();
    if ( ImGui::IsAnyItemActive() && !io.KeyCtrl ) return;

    if ( io.KeyCtrl ) {
        if ( ImGui::IsKeyPressed(ImGuiKey_N) ) {}
        if ( ImGui::IsKeyPressed(ImGuiKey_O) ) {
            openFolderPicker();
        }
        if ( ImGui::IsKeyPressed(ImGuiKey_S) ) {
            dispatchCommand(Logic::CmdSaveBeatmap{});
        }
        if ( ImGui::IsKeyPressed(ImGuiKey_Z) ) {
            dispatchCommand(Logic::CmdUndo{});
        }
        if ( ImGui::IsKeyPressed(ImGuiKey_Y) ) {
            dispatchCommand(Logic::CmdRedo{});
        }
        if ( ImGui::IsKeyPressed(ImGuiKey_C) ) {
            dispatchCommand(Logic::CmdCopy{});
        }
        if ( ImGui::IsKeyPressed(ImGuiKey_V) ) {
            dispatchCommand(Logic::CmdPaste{});
        }
        if ( ImGui::IsKeyPressed(ImGuiKey_X) ) {
            dispatchCommand(Logic::CmdCut{});
        }
    } else {
        if ( ImGui::IsKeyPressed(ImGuiKey_Space) ) {}
    }
}

void MainMenuView::openFolderPicker()
{
    auto& config = Config::AppConfig::instance().getEditorSettings();
    if ( config.filePickerStyle == Config::FilePickerStyle::Native ) {
        nfdu8char_t* outPath = nullptr;
        nfdresult_t  result  = NFD_PickFolder(&outPath, nullptr);

        if ( result == NFD_OKAY ) {
            Event::OpenProjectEvent ev;
            ev.m_projectPath = outPath;
            Event::EventBus::instance().publish(ev);
            NFD_FreePath(outPath);
        }
        // Error handling omitted for brevity as per existing logic
    } else {
        IGFD::FileDialogConfig fdConfig;
        fdConfig.path              = ".";
        fdConfig.countSelectionMax = 1;
        fdConfig.flags             = ImGuiFileDialogFlags_Default;
        ImGuiFileDialog::Instance()->OpenDialog(
            "ProjectFolderPicker",
            TR("ui.file_manager.open_directory"),
            nullptr,
            fdConfig);
    }
}

void MainMenuView::update()
{
    handleHotkeys();

    Config::SkinManager& skinCfg = Config::SkinManager::instance();

    auto MenuItemWithFontIcon = [&skinCfg](const char* icon,
                                           const char* label,
                                           const char* shortcut = nullptr,
                                           bool        enabled = true) -> bool {
        Config::Color iconColor = skinCfg.getColor("icon");
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            ImVec4(iconColor.r, iconColor.g, iconColor.b, iconColor.a));

        // 调整间距：图标与文本间隔约半个字符宽
        float gap = ImGui::CalcTextSize(" ").x * 0.5f;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(gap, 0));

        // 使用 ImGui 内部 API 以实现完美的图标与文本对齐
        // MenuItemEx 会自动处理图标列的宽度，即使 icon 为 nullptr
        // 也能保持文本对齐
        bool clicked = ImGui::MenuItemEx(label, icon, shortcut, false, enabled);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        return clicked;
    };

    ImFont* menuFont = skinCfg.getFont("menu");
    if ( menuFont ) ImGui::PushFont(menuFont);

    if ( ImGui::BeginMenu(TR("ui.file")) ) {
        if ( MenuItemWithFontIcon(
                 ICON_MMM_BOOK, TR("ui.file.new_pro"), "Ctrl+N") ) {}
        if ( MenuItemWithFontIcon(ICON_MMM_FILE, TR("ui.file.new_map")) ) {}
        ImGui::Separator();

        if ( MenuItemWithFontIcon(
                 ICON_MMM_FOLDER_OPEN, TR("ui.file.open_pro"), "Ctrl+O") ) {
            openFolderPicker();
        }

        if ( MenuItemWithFontIcon(nullptr, TR("ui.file.open_recent")) ) {}
        ImGui::Separator();

        if ( MenuItemWithFontIcon(
                 ICON_MMM_SAVE, TR("ui.file.save"), "Ctrl+S") ) {
            dispatchCommand(Logic::CmdSaveBeatmap{});
        }
        if ( MenuItemWithFontIcon(nullptr, TR("ui.file.save_as")) ) {}
        ImGui::EndMenu();
    }
    if ( ImGui::BeginMenu(TR("ui.edit")) ) {
        if ( MenuItemWithFontIcon(
                 ICON_MMM_UNDO, TR("ui.edit.undo"), "Ctrl+Z") ) {
            dispatchCommand(Logic::CmdUndo{});
        }
        if ( MenuItemWithFontIcon(
                 ICON_MMM_REDO, TR("ui.edit.redo"), "Ctrl+Y") ) {
            dispatchCommand(Logic::CmdRedo{});
        }
        ImGui::Separator();
        if ( MenuItemWithFontIcon(
                 ICON_MMM_SCISSORS, TR("ui.edit.cut"), "Ctrl+X") ) {
            dispatchCommand(Logic::CmdCut{});
        }
        if ( MenuItemWithFontIcon(
                 ICON_MMM_COPY, TR("ui.edit.copy"), "Ctrl+C") ) {
            dispatchCommand(Logic::CmdCopy{});
        }
        if ( MenuItemWithFontIcon(nullptr, TR("ui.edit.paste"), "Ctrl+V") ) {
            dispatchCommand(Logic::CmdPaste{});
        }
        ImGui::Separator();
        if ( MenuItemWithFontIcon(
                 ICON_MMM_PLAY, TR("ui.edit.play_pause"), "Space") ) {}
        ImGui::EndMenu();
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text(
        "%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    if ( menuFont ) ImGui::PopFont();
}

}  // namespace MMM::UI

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
    // 只有在没有文本输入激活时才处理快捷键，除非是 Ctrl 组合键
    if ( ImGui::IsAnyItemActive() && !io.KeyCtrl ) return;

    if ( io.KeyCtrl ) {
        if ( ImGui::IsKeyPressed(ImGuiKey_N) ) {
            // New project logic placeholder
        }
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
        if ( ImGui::IsKeyPressed(ImGuiKey_Space) ) {
            // Toggle play state placeholder
        }
    }
}

void MainMenuView::openFolderPicker()
{
    auto& config = Config::AppConfig::instance().getEditorSettings();
    XINFO("Opening folder picker from MainMenu, style: {}",
          config.filePickerStyle == Config::FilePickerStyle::Native
              ? "Native"
              : "Unified");

    if ( config.filePickerStyle == Config::FilePickerStyle::Native ) {
        nfdu8char_t* outPath = nullptr;
        nfdresult_t  result  = NFD_PickFolder(&outPath, nullptr);

        if ( result == NFD_OKAY ) {
            Event::OpenProjectEvent ev;
            ev.m_projectPath = outPath;
            Event::EventBus::instance().publish(ev);
            NFD_FreePath(outPath);
        } else if ( result == NFD_CANCEL ) {
            XINFO("User cancelled native folder picker.");
        } else {
            XERROR("NFD Error: {}", NFD_GetError());
        }
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
        // 应用图标颜色
        Config::Color iconColor = skinCfg.getColor("icon");
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            ImVec4(iconColor.r, iconColor.g, iconColor.b, iconColor.a));

        // 添加前导空格为图标留出位置
        std::string padded_label = std::string(icon) + "    " + label;
        bool        clicked =
            ImGui::MenuItem(padded_label.c_str(), shortcut, false, enabled);

        ImGui::PopStyleColor();
        return clicked;
    };

    // 应用菜单字体
    ImFont* menuFont = skinCfg.getFont("menu");
    if ( menuFont ) ImGui::PushFont(menuFont);

    if ( ImGui::BeginMenu(TR("ui.file")) ) {
        if ( MenuItemWithFontIcon(
                 ICON_MMM_FILE_ADD, TR("ui.file.new_pro"), "Ctrl+N") ) {
            // New project
        }
        if ( MenuItemWithFontIcon(ICON_MMM_FILE, TR("ui.file.new_map")) ) {
            // New map
        }
        ImGui::Separator();

        if ( MenuItemWithFontIcon(
                 ICON_MMM_FOLDER_OPEN, TR("ui.file.open_pro"), "Ctrl+O") ) {
            openFolderPicker();
        }

        if ( ImGui::MenuItem(TR("ui.file.open_recent")) ) {
            /// 遍历最近打开过的项目
        }
        ImGui::Separator();

        if ( MenuItemWithFontIcon(
                 ICON_MMM_SAVE, TR("ui.file.save"), "Ctrl+S") ) {
            dispatchCommand(Logic::CmdSaveBeatmap{});
        }
        if ( ImGui::MenuItem(TR("ui.file.save_as")) ) {}
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
        if ( MenuItemWithFontIcon(
                 ICON_MMM_PASTE, TR("ui.edit.paste"), "Ctrl+V") ) {
            dispatchCommand(Logic::CmdPaste{});
        }
        ImGui::Separator();
        if ( MenuItemWithFontIcon(
                 ICON_MMM_PLAY, TR("ui.edit.play_pause"), "Space") ) {
            // Toggle play
        }
        ImGui::EndMenu();
    }
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text(
        "%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    if ( menuFont ) ImGui::PopFont();
}

}  // namespace MMM::UI

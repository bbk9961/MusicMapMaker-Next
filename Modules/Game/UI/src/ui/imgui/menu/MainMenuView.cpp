#include "ui/imgui/menu/MainMenuView.h"
#include "config/AppConfig.h"
#include "config/skin/SkinConfig.h"
#include "event/core/EventBus.h"
#include "event/ui/menu/OpenProjectEvent.h"
#include "log/colorful-log.h"
#include "ui/Icons.h"
#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <nfd.h>

namespace MMM::UI
{

MainMenuView::MainMenuView() {}

MainMenuView::~MainMenuView() {}

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
    Config::SkinManager& skinCfg = Config::SkinManager::instance();

    auto MenuItemWithFontIcon = [&skinCfg](const char* icon,
                                           const char* label) -> bool {
        // 应用图标颜色
        Config::Color iconColor = skinCfg.getColor("icon");
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            ImVec4(iconColor.r, iconColor.g, iconColor.b, iconColor.a));

        // 添加前导空格为图标留出位置
        std::string padded_label = std::string(icon) + "    " + label;
        bool        clicked      = ImGui::MenuItem(padded_label.c_str());

        ImGui::PopStyleColor();
        return clicked;
    };

    // 应用菜单字体 (由 MainDockSpaceUI
    // 统一推送，但这里也确保一下安全，或者在具体内容处推送)
    ImFont* menuFont = skinCfg.getFont("menu");
    if ( menuFont ) ImGui::PushFont(menuFont);

    if ( ImGui::BeginMenu(TR("ui.file")) ) {
        if ( ImGui::MenuItem(TR("ui.file.new_map")) ) {}
        if ( ImGui::MenuItem(TR("ui.file.new_pro")) ) {}
        ImGui::Separator();

        if ( ImGui::MenuItem(TR("ui.file.open_map")) ) {}

        if ( MenuItemWithFontIcon(ICON_MMM_FOLDER_OPEN,
                                  TR("ui.file.open_pro")) ) {
            openFolderPicker();
        }

        if ( ImGui::MenuItem(TR("ui.file.open_recent")) ) {
            /// 遍历最近打开过的项目
        }
        ImGui::Separator();

        if ( ImGui::MenuItem(TR("ui.file.save")) ) {}
        if ( ImGui::MenuItem(TR("ui.file.save_as")) ) {}
        ImGui::EndMenu();
    }
    if ( ImGui::BeginMenu(TR("ui.edit")) ) {
        ImGui::EndMenu();
    }
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text(
        "%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    if ( menuFont ) ImGui::PopFont();
}

}  // namespace MMM::UI

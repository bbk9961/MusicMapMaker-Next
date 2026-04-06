#include "ui/imgui/menu/MainMenuView.h"
#include "config/AppConfig.h"
#include "config/skin/SkinConfig.h"
#include "event/core/EventBus.h"
#include "event/ui/menu/OpenProjectEvent.h"
#include "log/colorful-log.h"
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
    auto MenuItemWithIcon = [](const char*         label,
                               Graphic::VKTexture* tex) -> bool {
        ImVec2 pos         = ImGui::GetCursorScreenPos();
        float  line_height = ImGui::GetTextLineHeight();

        // 添加前导空格为图标留出位置
        std::string padded_label = std::string("      ") + label;
        bool        clicked      = ImGui::MenuItem(padded_label.c_str());

        if ( tex ) {
            float  icon_size = line_height * 1.2f;
            float  offset_y  = (ImGui::GetItemRectSize().y - icon_size) * 0.5f;
            ImVec2 p_min     = ImVec2(pos.x + 8.0f, pos.y + offset_y);
            ImVec2 p_max     = ImVec2(p_min.x + icon_size, p_min.y + icon_size);
            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)tex->getImTextureID(), p_min, p_max);
        }
        return clicked;
    };

    if ( ImGui::BeginMenu(TR("ui.file")) ) {
        if ( ImGui::MenuItem(TR("ui.file.new_map")) ) {}
        if ( ImGui::MenuItem(TR("ui.file.new_pro")) ) {}
        ImGui::Separator();

        if ( ImGui::MenuItem(TR("ui.file.open_map")) ) {}

        if ( MenuItemWithIcon(TR("ui.file.open_pro"), m_folderIcon.get()) ) {
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
}

}  // namespace MMM::UI

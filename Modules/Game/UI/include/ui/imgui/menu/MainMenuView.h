#pragma once

#include "graphic/imguivk/VKTexture.h"
#include <memory>

namespace MMM::UI
{

class MainMenuView
{
public:
    MainMenuView();
    MainMenuView(MainMenuView&&)                 = default;
    MainMenuView(const MainMenuView&)            = delete;
    MainMenuView& operator=(MainMenuView&&)      = default;
    MainMenuView& operator=(const MainMenuView&) = delete;
    ~MainMenuView();

    void update();

    void setOpenProjectIcon(std::unique_ptr<Graphic::VKTexture> tex)
    {
        m_openProjectIcon = std::move(tex);
    }

    void setFolderIcon(std::unique_ptr<Graphic::VKTexture> tex)
    {
        m_folderIcon = std::move(tex);
    }

    void setSaveIcon(std::unique_ptr<Graphic::VKTexture> tex)
    {
        m_saveIcon = std::move(tex);
    }

    void setPlusIcon(std::unique_ptr<Graphic::VKTexture> tex)
    {
        m_plusIcon = std::move(tex);
    }

private:
    std::unique_ptr<Graphic::VKTexture> m_openProjectIcon;
    std::unique_ptr<Graphic::VKTexture> m_folderIcon;
    std::unique_ptr<Graphic::VKTexture> m_saveIcon;
    std::unique_ptr<Graphic::VKTexture> m_plusIcon;

    void openFolderPicker();
};

}  // namespace MMM::UI

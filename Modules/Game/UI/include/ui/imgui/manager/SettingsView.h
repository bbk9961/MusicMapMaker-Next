#pragma once

#include "graphic/imguivk/VKTexture.h"
#include "ui/ISubView.h"
#include "ui/ITextureLoader.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace MMM::UI
{

enum class SettingsTab {
    Software,  // 软件配置
    Visual,    // 视觉配置
    Project,   // 项目配置
    Editor     // 编辑器配置
};

class SettingsView : public ISubView, public ITextureLoader
{
public:
    SettingsView(const std::string& subViewName);
    SettingsView(SettingsView&&)                 = default;
    SettingsView(const SettingsView&)            = default;
    SettingsView& operator=(SettingsView&&)      = delete;
    SettingsView& operator=(const SettingsView&) = delete;
    ~SettingsView() override                     = default;

    void onUpdate(LayoutContext& layoutContext,
                  UIManager*     sourceManager) override;

    void update(UIManager* sourceManager) override;

    /// @brief 是否需要重载
    bool needReload() override { return m_needReload; }

    /// @brief 重载纹理
    void reloadTextures(vk::PhysicalDevice& physicalDevice,
                        vk::Device& logicalDevice, vk::CommandPool& cmdPool,
                        vk::Queue& queue) override;

private:
    SettingsTab m_currentTab = SettingsTab::Software;
    bool        m_needReload = true;

    std::unordered_map<SettingsTab, std::unique_ptr<Graphic::VKTexture>>
        m_tabIcons;

    void drawSoftwareSettings();
    void drawVisualSettings();
    void drawProjectSettings();
    void drawEditorSettings();
};

}  // namespace MMM::UI

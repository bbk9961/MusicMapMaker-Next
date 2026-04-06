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

class SettingsView : public ISubView
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

private:
    SettingsTab m_currentTab = SettingsTab::Software;

    void drawSoftwareSettings();
    void drawVisualSettings();
    void drawProjectSettings();
    void drawEditorSettings();
};

}  // namespace MMM::UI

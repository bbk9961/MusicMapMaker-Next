#pragma once

#include "graphic/imguivk/VKTexture.h"
#include "ui/ISubView.h"
#include "ui/ITextureLoader.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "event/ui/UISettingsTabEvent.h"

namespace MMM::UI
{


class SettingsView : public ISubView
{
public:
    SettingsView(const std::string& subViewName);
    SettingsView(SettingsView&&)                 = default;
    SettingsView(const SettingsView&)            = default;
    SettingsView& operator=(SettingsView&&)      = delete;
    SettingsView& operator=(const SettingsView&) = delete;
    ~SettingsView() override;

    void onUpdate(LayoutContext& layoutContext,
                  UIManager*     sourceManager) override;

private:
    Event::SettingsTab m_currentTab = Event::SettingsTab::Software;
    uint64_t m_tabSubId = 0;

    void drawSoftwareSettings();
    void drawVisualSettings();
    void drawProjectSettings();
    void drawBeatmapSettings();
    void drawEditorSettings();
};

}  // namespace MMM::UI

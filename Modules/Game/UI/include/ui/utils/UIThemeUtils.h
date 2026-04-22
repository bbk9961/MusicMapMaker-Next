#pragma once

#include <imgui.h>

namespace MMM::UI::Utils {

/**
 * @brief Utility for UI semantic colors based on the current ImGui theme (light/dark context)
 */
class UIThemeUtils {
public:
    static ImVec4 getDangerColor() {
        return {1.0f, 0.2f, 0.2f, 1.0f}; // Consistent across themes for now, can be tweaked later
    }

    static ImVec4 getWarningColor() {
        return {1.0f, 0.8f, 0.0f, 1.0f};
    }

    static ImVec4 getHighlightColor() {
        // A nice blue highlight
        return {0.2f, 0.6f, 1.0f, 1.0f};
    }
    
    static ImVec4 getDisabledColor() {
        // Grab text color and make it transparent to fake "disabled" look
        ImVec4 textCol = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        return {textCol.x, textCol.y, textCol.z, textCol.w * 0.5f};
    }

    // Helper to make a transparent button by grabbing the current theme's text color
    // and using small alphas for hover/active states.
    static void pushTransparentButtonStyles() {
        ImVec4 textCol = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(textCol.x, textCol.y, textCol.z, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(textCol.x, textCol.y, textCol.z, 0.2f));
    }

    static void popTransparentButtonStyles() {
        ImGui::PopStyleColor(3);
    }
};

} // namespace MMM::UI::Utils

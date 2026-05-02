#pragma once

#include "EditorSettings.h"
#include "VisualConfig.h"

#include <string>
#include <vector>

namespace MMM::Config
{

/// @brief 编辑器全局配置容器 (持久化于 user_config.json)
/// @details 包含视觉表现和编辑行为两大类配置
struct EditorConfig {
    /// @brief 视觉、布局与渲染相关的配置
    VisualConfig visual;

    /// @brief 编辑逻辑、同步与音效策略相关的设置
    EditorSettings settings;

    /// @brief 最近打开的项目路径列表
    std::vector<std::string> recentProjects;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EditorConfig, visual, settings,
                                   recentProjects)
};

}  // namespace MMM::Config

#pragma once

namespace MMM::Logic
{

/**
 * @brief 交互状态组件
 *
 * 存储 ECS 实体的 UI 交互状态（如悬停、选中、拖拽中等），由逻辑线程基于 UI
 * 线程的指令进行更新。
 */
struct InteractionComponent {
    bool isHovered{ false };
    bool isSelected{ false };
    bool isDragging{ false };
};

}  // namespace MMM::Logic

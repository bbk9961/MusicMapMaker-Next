#pragma once

#include <clocale>
#include <filesystem>
#include <iostream>

// 平台特定头文件
#ifdef _WIN32
#    include <windows.h>
#elif __APPLE__
#    include <limits.h>
#    include <mach-o/dyld.h>
#else  // Linux
#    include <limits.h>
#    include <unistd.h>
#endif

#include "log/colorful-log.h"

namespace MMM
{

/// @brief 正常退出码
constexpr int EXIT_NORMAL = 0;

/// @brief 窗口异常退出码
constexpr int EXIT_WINDOW_EXEPTION = 1;

/**
 * @brief RAII 日志管理器
 *
 * 利用静态对象的生命周期自动初始化和关闭日志系统。
 */
struct RTTILogger {
    RTTILogger()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);  // 强制当前控制台输出为 UTF-8
#endif
        std::setlocale(LC_ALL, ".UTF-8");
        namespace fs = std::filesystem;
        // 2. 设置工作目录为可执行文件所在目录
        try {
            fs::path exePath;

#if defined(_WIN32)
            wchar_t buffer[MAX_PATH];
            GetModuleFileNameW(NULL, buffer, MAX_PATH);
            exePath = fs::path(buffer);

#elif defined(__APPLE__)
            char     buffer[PATH_MAX];
            uint32_t size = sizeof(buffer);
            if ( _NSGetExecutablePath(buffer, &size) == 0 ) {
                exePath = fs::path(buffer);
            } else {
                // 如果路径太长，动态分配缓冲区（极少见）
                std::string longPath(size, '\0');
                _NSGetExecutablePath(longPath.data(), &size);
                exePath = fs::path(longPath);
            }

#else  // Linux
            char    buffer[PATH_MAX];
            ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
            if ( count != -1 ) {
                exePath = fs::path(std::string(buffer, count));
            }
#endif

            if ( !exePath.empty() ) {
                // 使用绝对路径并切换工作目录
                fs::current_path(fs::absolute(exePath).parent_path());
            }
        } catch ( const std::exception& e ) {
            std::cerr << "Warning: Failed to set working directory: "
                      << e.what() << std::endl;
        }
        XLogger::init("MMM");
    }

    ~RTTILogger() { XLogger::shutdown(); }
};

/// @brief 全局日志管理器实例 (程序启动时自动初始化)
inline RTTILogger rttiLogger{};

}  // namespace MMM

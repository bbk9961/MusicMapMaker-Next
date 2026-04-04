#include "mmm/beatmap/BeatMap.h"

#include "LoadMalodyMap.hpp"
#include "LoadOSUMap.hpp"
#include "LoadRMMap.hpp"

#include "log/colorful-log.h"
#include <filesystem>

namespace MMM
{

/**
 * @brief 从文件加载谱面
 * @param mapFilePath 谱面文件路径
 */
BeatMap BeatMap::loadFromFile(std::filesystem::path mapFilePath)
{
    if ( !std::filesystem::exists(mapFilePath) ) {
        XWARN("Load Map Failed: File Not Exists.");
        return {};
    }
    if ( !mapFilePath.has_extension() ) {
        XWARN("Load Map Failed: Unknown File extension.");
        return {};
    }
    std::string mapFileExtention = mapFilePath.extension().generic_string();
    if ( mapFileExtention == ".osu" ) {
        return loadOSUMap(mapFilePath);
    }
    if ( mapFileExtention == ".mc" ) {
        return loadMalodyMap(mapFilePath);
    }
    if ( mapFileExtention == ".imd" ) {
        return loadRMMap(mapFilePath);
    }
    XWARN("Unsupport map file type: {}", mapFileExtention);
    return {};
}

BeatMap::BeatMap() {}

BeatMap::~BeatMap() {}
}  // namespace MMM

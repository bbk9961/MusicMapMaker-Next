#include "mmm/note/Note.h"
#include <cmath>

namespace MMM
{
Note::Note() {}

Note::~Note() {}

/// @brief 从osu描述加载
void Note::from_osu_description(const std::vector<std::string>& description,
                                int32_t                         orbit_count)
{
    using enum NoteMetadataType;
    auto& osunote_prop = m_metadata.note_properties[OSU];
    m_type             = NoteType::NOTE;

    /*
     *长键（仅 osu!mania）
     *长键语法： x,y,开始时间,物件类型,长键音效,结束时间,长键音效组
     *
     *结束时间（整型）： 长键的结束时间，以谱面音频开始为原点，单位是毫秒。
     *x 与长键所在的键位有关。算法为：floor(x * 键位总数 / 512)，并限制在 0 和
     *键位总数 - 1 之间。 *y 不影响长键。默认值为 192，即游戏区域的水平中轴。
     */

    // 位置
    m_track = uint32_t(
        std::floor(std::stod(description.at(0)) * double(orbit_count) / 512.));

    // 没卵用-om固定192
    // int y = std::stoi(description.at(1));

    // 时间戳
    m_timestamp = std::stoi(description.at(2));

    // 音效
    osunote_prop["sample"] = std::stoi(description.at(4));

    // 音效组
    osunote_prop["samplegroup"] = description.at(5);
}

/// @brief 转换为osu描述
std::string Note::to_osu_description(int32_t orbit_count)
{
    using enum NoteMetadataType;
    auto& osunote_prop = m_metadata.note_properties[OSU];
    /*
     * 格式:
     * x,y,开始时间,物件类型,长键音效,结束时间:音效组:附加音效组:音效参数:音量[:自定义音效文件]
     * 对于单键:
     *   - 结束时间 = 开始时间
     *   - 音效组参数格式为:
     * normalSet:additionalSet:sampleSetParameter:volume:[sampleFile]
     */

    std::ostringstream oss;

    // x 坐标 (根据轨道数计算)
    // 原公式: orbit = floor(x * orbit_count / 512)
    // 反推: x = orbit * 512 / orbit_count
    auto x = static_cast<int>((double(m_track) + 0.5) * 512 / orbit_count);
    oss << x << ",";

    // y 坐标 (固定192)
    oss << "192,";

    // 开始时间
    oss << m_timestamp << ",";

    // 物件类型 (NOTE=1)
    oss << "1,";

    // 长键音效 (NoteSample枚举值)
    if ( auto it = osunote_prop.find("sample"); it != osunote_prop.end() ) {
        oss << it->second << ",";
    } else {
        oss << "0" << ",";
    }

    // 结束时间 (单键等于开始时间)
    oss << m_timestamp << ":";

    // 音效组参数
    if ( auto it = osunote_prop.find("samplegroup");
         it != osunote_prop.end() ) {
        std::string notegroup = it->second;
        if ( auto it_pos = std::ranges::find(notegroup, ':');
             it_pos != notegroup.end() ) {
            // 从头删除到冒号的下一个位置
            notegroup.erase(notegroup.begin(), std::next(it_pos));
        }
        oss << notegroup << ",";
    } else {
        oss << "0:0:0:" << ",";
    }

    return oss.str();
}
}  // namespace MMM

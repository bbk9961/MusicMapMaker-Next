#include "FormatTestHelpers.hpp"
#include "TestHelper.hpp"
#include "mmm/beatmap/BeatMap.h"

static constexpr int TOTAL_OSU_SECTIONS = 7;

int main(int argc, char* argv[])
{
    if ( argc < 3 ) return 1;
    std::filesystem::path input  = argv[1];
    std::filesystem::path output = argv[2];

    XINFO("========================================");
    XINFO("  OSU Consistency Test: {}", input.filename().string());
    XINFO("========================================");

    // ── 第一轮：文本分章节对比 ──
    MMM::BeatMap m1 = MMM::BeatMap::loadFromFile(input);
    m1.sync();

    if ( !m1.saveToFile(output) ) return 1;

    MMM::BeatMap m2 = MMM::BeatMap::loadFromFile(output);
    m2.sync();

    // 文本分章节对比
    int sectionPassed = MMM::Test::compareOSUSections(input, output);
    XINFO("OSU Text Section Comparison: {}/{} sections passed",
          sectionPassed,
          TOTAL_OSU_SECTIONS);

    // ── 第二轮：逻辑一致性对比 ──
    bool logicPassed = MMM::Test::compareBeatMaps(m1, m2);
    if ( logicPassed ) {
        XINFO("[OSU Logical Consistency]: PASS");
    } else {
        XERROR("[OSU Logical Consistency]: FAIL");
    }

    // ── 汇总 ──
    int totalPassed = sectionPassed + (logicPassed ? 1 : 0);
    int totalTests  = TOTAL_OSU_SECTIONS + 1;
    XINFO("========================================");
    if ( sectionPassed == TOTAL_OSU_SECTIONS && logicPassed ) {
        XINFO("  OSU Consistency: ALL {}/{} PASSED", totalPassed, totalTests);
        return 0;
    } else {
        XERROR("  OSU Consistency: {}/{} passed, {} failed",
               totalPassed,
               totalTests,
               totalTests - totalPassed);
        return 1;
    }
}

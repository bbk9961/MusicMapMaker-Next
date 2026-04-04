add_library(MMM
    STATIC
    src/beatmap/BeatMap.cpp
    src/note/Note.cpp
    src/note/Hold.cpp
    src/timing/Timing.cpp
)

target_include_directories(MMM PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(MMM PUBLIC Log Event)

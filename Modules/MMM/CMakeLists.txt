add_library(MMM
    STATIC
    src/beatmap/BeatMap.cpp
    src/note/Note.cpp
)

target_include_directories(MMM PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

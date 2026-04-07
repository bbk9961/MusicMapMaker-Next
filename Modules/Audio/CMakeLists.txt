# Modules/Audio/CMakeLists.txt

add_library(Audio STATIC
	src/AudioManager.cpp
	src/SoundEffectPool.cpp
)

target_link_libraries(Audio
	PUBLIC
		ICE::IonCachyEngine
		Log
		3rd_rubberband
)
if(MSVC)
	target_link_libraries(Audio
		PUBLIC
			3rd_rubberband
			fftw3
			samplerate
			SDL3-static
	)
endif()

target_include_directories(Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

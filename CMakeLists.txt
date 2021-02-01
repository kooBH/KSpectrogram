# CMAKE version of Ubuntu 16.04 is 3.5.
cmake_minimum_required(VERSION 3.5)
project(Spectrogram)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LINKLIBS)
set(COMPILE_OPTION)
set(SRC)
set(INCL)
  
if(NOT MSVC)
	list(APPEND LINKLIBS
		m
		-fopenmp
	)
	list(APPEND COMPILE_OPTION
		-fopenmp
		-O3
	)
else()
	list(APPEND COMPILE_OPTION
		/openmp
		# /MP - compiled in mult-process : faster build time
		/MP
	)
endif()

include(lib/RtAudioWrapper/RtAudio.cmake)
include(lib/RtAudioWrapper/Libsamplerate.cmake)

list(APPEND SRC
	src/main.cpp
)
	
list(APPEND INCL
	src
	
	lib/RtAudioWrapper
	lib/STFT/cpp
	lib/WAV
	lib/verdigris/src
	
	# QtFramework
	bin/include
)

## QtFramework ##
if(UNIX AND NOT APPLE)
  list(APPEND LINKLIBS
	)
  list(APPEND COMPILE_OPTION
      -fPIC
    )
elseif(WIN32)
	list(APPEND LINKLIBS
	)
endif()

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME}	PRIVATE	${INCL}  )
target_link_libraries(${PROJECT_NAME} ${LINKLIBS})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION} )
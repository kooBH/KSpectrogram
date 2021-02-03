# CMAKE version of Ubuntu 16.04 is 3.5.
cmake_minimum_required(VERSION 3.5)
project(KSpectrogram)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LINKLIBS)
set(COMPILE_OPTION)
set(SRC)
set(SRC_Qt_MOC)
set(SRC_Qt)
set(INCL)

macro(display_elements
	DISPLAY_MESSAGE
	LIST_TO_DISPLAY)
    message("[ ${DISPLAY_MESSAGE} ]")
    foreach(_DISPLAY_ELEMENT ${LIST_TO_DISPLAY})
	message(STATUS ${_DISPLAY_ELEMENT})
    endforeach()
endmacro()

## MOC(Meta Object Compiler) of QtFramework

	## sources to be processed by MOC
	list(APPEND SRC_Qt_MOC
		src/KSpectrogram.h
		src/KAnalysis.h
		
	)
	
	## sources without MetaObject 
	list(APPEND SRC_Qt
		src/KSpecWidget.h
		
		src/KSpectrogram.cpp
		src/KAnalysis.cpp
		src/KSpecWidget.cpp
		src/BorderLayout.h
		src/BorderLayout.cpp
		)
	
	foreach(i  ${SRC_Qt_MOC})
		string(REPLACE "/" ";" parse_i_1 ${i})
		#message(${parse_i_1})
		LIST(GET parse_i_1 1 parse_i_2)
		#message(${parse_i_2})
		string(REPLACE "." ";" parse_i_3 ${parse_i_2})
		#message(${parse_i_3})
		LIST(GET parse_i_3 0 fisrt_element)
		#message(${fisrt_element})
		if(MSVC)
			execute_process(
			COMMAND moc ../${i} -o ../tmp/moc_${fisrt_element}.cpp
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin  
			
			 )
		endif()
		
		list(APPEND SRC_Qt
			../tmp/moc_${fisrt_element}.cpp
		)
	endforeach()

	#list(APPEND COMPILE_OPTION -D_MOC_)
################

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
		#/openmp
		# /MP - compiled in mult-process : faster build time
		/MP
	)
endif()

include(lib/RtAudioWrapper/RtAudio.cmake)
include(lib/RtAudioWrapper/Libsamplerate.cmake)

list(APPEND SRC
	src/main.cpp
	
	src/LogSpec.h
	src/ColorMap.h
)

list(APPEND SRC ${SRC_Qt} ${SRC_Qt_MOC})
	
list(APPEND INCL
	src
	tmp
	
	lib/RtAudioWrapper
	lib/STFT/cpp
	lib/WAV
	lib/verdigris/src
	
	# QtFramework
	bin/include
	bin/include/QtCore
	bin/include/QtCore5Compat
	bin/include/QtGui
	bin/include/QtWidgets
	bin/include/QtOpenGL
	bin/include/QtOpenGLWidgets
)

## QtFramework ##
if(UNIX AND NOT APPLE)
  list(APPEND LINKLIBS
	)
  list(APPEND COMPILE_OPTION
      -fPIC
    )
elseif(MSVC)
	list(APPEND LINKLIBS
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Cored.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Core.lib
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Guid.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Gui.lib
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Widgetsd.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Widgets.lib	
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6OpenGLWidgetsd.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6OpenGLWidgets.lib	
	)
endif()

display_elements(SRC "${SRC}")

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME}	PRIVATE	${INCL}  )
target_link_libraries(${PROJECT_NAME} ${LINKLIBS})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION} )
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

## MOC(Meta Object Compiler) of QtFramework

	## sources to be processed by MOC
	list(APPEND SRC_Qt_MOC
		src/KSpectrogram.h
		src/KAnalysis.h
		src/KSpecWidget.h
	)
	
	## sources without MetaObject 
	list(APPEND SRC_Qt
		src/KSpectrogram.cpp
		src/KAnalysis.cpp
		src/KSpecWidget.cpp
		src/BorderLayout.h
		src/BorderLayout.cpp
		)
	
	foreach(i  ${SRC_Qt_MOC})
		string(REPLACE "/" ";" parse_i ${i})
		LIST(GET parse_i 1 second_element)
		
		if(MSVC)
			execute_process(
			COMMAND moc ../${i} -o ../tmp/moc_${second_element} -D pragma
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin  
			
			 )
		endif()
		
		list(APPEND SRC_Qt
			../tmp/moc_${second_element}
		)
	endforeach()

	list(APPEND COMPILE_OPTION -D_MOC_)
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
		/openmp
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

message(${LINKLIBS})

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME}	PRIVATE	${INCL}  )
target_link_libraries(${PROJECT_NAME} ${LINKLIBS})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION} )
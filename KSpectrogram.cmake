################################################################
# CMake file for using KSpectrogram as submodule               #
# 															   #
################################################################

list(APPEND SRC_Qt_MOC
	${CMAKE_CURRENT_LIST_DIR}/src/KSpectrogram.h
	${CMAKE_CURRENT_LIST_DIR}/src/KAnalysis.h
)

list(APPEND SRC_Qt
	${CMAKE_CURRENT_LIST_DIR}/src/KSpecWidget.h
	${CMAKE_CURRENT_LIST_DIR}/src/KSpectrogram.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/KAnalysis.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/KSpecWidget.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/BorderLayout.h
	${CMAKE_CURRENT_LIST_DIR}/src/BorderLayout.cpp
)

list(APPEND SRC_Qt
	${CMAKE_CURRENT_LIST_DIR}/src/LogSpec.h
	${CMAKE_CURRENT_LIST_DIR}/src/ColorMap.h
	)
	
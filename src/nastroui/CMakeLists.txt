cmake_minimum_required(VERSION 3.26.4)

project(NASTROUI VERSION 0.0.1 LANGUAGES CXX)

####
# Build
####

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

file(GLOB NASTROUI_Resources CONFIGURE_DEPENDS res/resources.qrc)
file(GLOB NASTROUI_SourceFiles CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB NASTROUI_UI_SourceFiles CONFIGURE_DEPENDS src/UI/*.cpp src/UI/*.h)
file(GLOB NASTROUI_VM_SourceFiles CONFIGURE_DEPENDS src/VM/*.cpp src/VM/*.h)
file(GLOB NASTROUI_Util_SourceFiles CONFIGURE_DEPENDS src/Util/*.cpp src/Util/*.h)
file(GLOB NASTROUI_Image_SourceFiles CONFIGURE_DEPENDS src/Image/*.cpp src/Image/*.h)
file(GLOB NASTROUI_Data_SourceFiles CONFIGURE_DEPENDS src/Data/*.cpp src/Data/*.h)
file(GLOB NASTROUI_ColorMaps_SourceFiles CONFIGURE_DEPENDS src/ColorMaps/*.cpp src/ColorMaps/*.h)

qt_add_executable(nastroui
	${NASTROUI_Resources}
	${NASTROUI_SourceFiles}
	${NASTROUI_UI_SourceFiles}
	${NASTROUI_VM_SourceFiles}
	${NASTROUI_Util_SourceFiles}
	${NASTROUI_Image_SourceFiles}
	${NASTROUI_Data_SourceFiles}
	${NASTROUI_ColorMaps_SourceFiles}
)

target_compile_features(nastroui
	PRIVATE
		cxx_std_23
)

target_compile_options(nastroui
	PRIVATE
		${NASTRO_WARNINGS_FLAGS}
)

target_link_libraries(nastroui
	PRIVATE
		NFITS
		Qt6::Gui Qt6::Widgets # Core already linked by qt_add_executable
)

set_target_properties(nastroui
	PROPERTIES
		INSTALL_RPATH "$ORIGIN/"
)

# On Windows, copy runtime dlls to same directory as the binary
if (CMAKE_IMPORT_LIBRARY_SUFFIX)
	add_custom_command(
		TARGET nastroui POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			-t $<TARGET_FILE_DIR:nastroui>
			$<TARGET_RUNTIME_DLLS:nastroui>
		COMMAND_EXPAND_LISTS
	)
endif()

####
# Installation
####

install(
    TARGETS
        nastroui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	FILES "licenses.txt"
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

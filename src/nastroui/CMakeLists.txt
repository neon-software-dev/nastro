cmake_minimum_required(VERSION 3.26.4)

project(NASTROUI VERSION 0.0.2 LANGUAGES CXX)

####
# Build
####

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Widgets Charts)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

file(GLOB NASTROUI_Resources CONFIGURE_DEPENDS res/resources.qrc)
file(GLOB NASTROUI_SourceFiles CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB NASTROUI_UI_SourceFiles CONFIGURE_DEPENDS src/UI/*.cpp src/UI/*.h)
file(GLOB NASTROUI_VM_SourceFiles CONFIGURE_DEPENDS src/VM/*.cpp src/VM/*.h)
file(GLOB NASTROUI_Util_SourceFiles CONFIGURE_DEPENDS src/Util/*.cpp src/Util/*.h)

qt_add_executable(nastroui
	${NASTROUI_Resources}
	${NASTROUI_SourceFiles}
	${NASTROUI_UI_SourceFiles}
	${NASTROUI_VM_SourceFiles}
	${NASTROUI_Util_SourceFiles}
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
		Qt6::Gui Qt6::Widgets Qt6::Charts # Core already linked by qt_add_executable
)

set_target_properties(nastroui
	PROPERTIES
		INSTALL_RPATH "$ORIGIN/"
)

# On Windows, copy runtime dlls to same directory as the binary
#if (CMAKE_IMPORT_LIBRARY_SUFFIX)
#	add_custom_command(
#		TARGET nastroui POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy
#			-t $<TARGET_FILE_DIR:nastroui>
#			$<TARGET_RUNTIME_DLLS:nastroui>
#		COMMAND_EXPAND_LISTS
#	)
#endif()

# On Windows, run windeployqt on the output file so to make Qt ready to run
if(WIN32)
	add_custom_command(
		TARGET nastroui POST_BUILD
		COMMAND Qt6::windeployqt
		ARGS $<TARGET_FILE:nastroui>
	)
endif()

# Qt deployment script to be executed at install time
qt_generate_deploy_script(
	TARGET nastroui
	OUTPUT_SCRIPT qt_deploy_script
	CONTENT "
		qt_deploy_runtime_dependencies(
			EXECUTABLE \"$<TARGET_FILE:nastroui>\"
			GENERATE_QT_CONF
			VERBOSE
		)"
)

####
# Installation
####

install(
    TARGETS
        nastroui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	FILES "nastroui_licenses.txt"
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	FILES $<TARGET_RUNTIME_DLLS:nastroui>
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (WIN32)
	# On Windows, execute the qt deploy script which runs windeployqt, among other things
	install(SCRIPT ${qt_deploy_script})

	# On Windows, also manually install all vcpkg-installed runtime files
	set(NASTRO_VCPKG_DIR "${CMAKE_SOURCE_DIR}/../external/vcpkg")

	set(NASTRO_INSTALLED_DIR "${NASTRO_VCPKG_DIR}/installed/x64-windows/bin/")
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(NASTRO_INSTALLED_DIR "${NASTRO_VCPKG_DIR}/installed/x64-windows/debug/bin/")
	endif()
	
	file(GLOB VCPKG_INSTALLED_RUNTIME "${NASTRO_INSTALLED_DIR}/*.dll")
	install(FILES ${VCPKG_INSTALLED_RUNTIME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

####
# CPack
####

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "nastroui")
set(CPACK_PACKAGE_VENDOR "NEON Software")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nastro FITS Viewer")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "none")

# Default installation directory on Windows
set(CPACK_PACKAGE_INSTALL_DIRECTORY "nastroui")

# What kind of packages to generate
if(WIN32)
	set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
	set(CPACK_GENERATOR "DragNDrop")
else()
	set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

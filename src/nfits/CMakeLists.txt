cmake_minimum_required(VERSION 3.26.4)

project(NFITS VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

####
# Global variables
####

add_compile_definitions(NFITS_DO_EXPORT)

####
# NFITS Lib
####

if (BUILD_TESTING)
	find_package(GTest CONFIG REQUIRED)
	include(CTest)
	enable_testing()
endif()

file(GLOB NFITS_PublicIncludes CONFIGURE_DEPENDS include/NFITS/*.h)
file(GLOB NFITS_Util_PublicIncludes CONFIGURE_DEPENDS include/NFITS/Util/*.h)
file(GLOB NFITS_Data_PublicIncludes CONFIGURE_DEPENDS include/NFITS/Data/*.h)
file(GLOB NFITS_Image_PublicIncludes CONFIGURE_DEPENDS include/NFITS/Image/*.h)

file(GLOB NFITS_SourceFiles CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB NFITS_Util_SourceFiles CONFIGURE_DEPENDS src/Util/*.cpp src/Util/*.h)
file(GLOB NFITS_Data_SourceFiles CONFIGURE_DEPENDS src/Data/*.cpp src/Data/*.h)
file(GLOB NFITS_Image_SourceFiles CONFIGURE_DEPENDS src/Image/*.cpp src/Image/*.h)
file(GLOB NFITS_ColorMaps_SourceFiles CONFIGURE_DEPENDS src/ColorMaps/*.cpp src/ColorMaps/*.h)
file(GLOB NFITS_Codec_SourceFiles CONFIGURE_DEPENDS src/Codec/*.cpp src/Codec/*.h)

add_library(NFITS
	${NFITS_PublicIncludes}
	${NFITS_Util_PublicIncludes}
	${NFITS_Data_PublicIncludes}
	${NFITS_Image_PublicIncludes}

	${NFITS_SourceFiles}
	${NFITS_Util_SourceFiles}
	${NFITS_Data_SourceFiles}
	${NFITS_Image_SourceFiles}
	${NFITS_ColorMaps_SourceFiles}
	${NFITS_Codec_SourceFiles}
)

if (BUILD_SHARED_LIBS)
	target_compile_definitions(NFITS PUBLIC NFITS_SHARED)
else()
	target_compile_definitions(NFITS PUBLIC NFITS_STATIC)
endif()

target_compile_features(NFITS
	PUBLIC
		cxx_std_23
	PRIVATE
		cxx_std_23
)

target_compile_options(NFITS
	PRIVATE
		${NASTRO_WARNINGS_FLAGS}
		${NASTRO_LIBRARY_FLAGS}
)

target_include_directories(NFITS
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(NFITS
	PROPERTIES
		OUTPUT_NAME NFITS
		EXPORT_NAME NFITS
		VERSION ${NFITS_VERSION}
		SOVERSION ${NFITS_VERSION_MAJOR}
)

####
# Tests executable
####

if (BUILD_TESTING)
	file(GLOB NFITS_Tests_SourceFiles CONFIGURE_DEPENDS tests/*.cpp)

	add_executable(libNFITSTests
		${NFITS_Tests_SourceFiles}
	)

	add_test(NAME libNFITSTests COMMAND libNFITSTests)

	# Give tests access to NFITS src content to allow for testing internal classes
	target_include_directories(libNFITSTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

	target_link_libraries(libNFITSTests
		PRIVATE
			NFITS
			GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
	)

	target_compile_features(libNFITSTests
		PRIVATE
			cxx_std_23
	)

	# Configure the installed executable to find NFITS in the lib directory
	set_target_properties(libNFITSTests
		PROPERTIES
			INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
	)

	# Copy the samples directory to the build output directory
	#add_custom_target(CopySamplesNFITSTests ALL
	#	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	#	"${CMAKE_CURRENT_SOURCE_DIR}/tests/samples/"
	#	"$<TARGET_FILE_DIR:libNFITSTests>/samples/"
	#	COMMENT "Copying samples directory to runtime output directory"
	#)
	#add_dependencies(CopySamplesNFITSTests libNFITSTests)
endif()

####
# Installation
#
####

# Temporary install if nfits is built as shared within nastro
if (BUILD_SHARED_LIBS)
	install(
		TARGETS
			NFITS
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

# Below is full install for when nfits is released by itself
#[[
set(NFITS_NAMESPACE            "NFITS::")
set(NFITS_PACKAGENAME          "NFITS")
set(NFITS_GENERATED_DIR        "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(NFITS_VERSION_CONFIG_FILE  "${NFITS_GENERATED_DIR}/${NFITS_PACKAGENAME}ConfigVersion.cmake")
set(NFITS_PROJECT_CONFIG_FILE  "${NFITS_GENERATED_DIR}/${NFITS_PACKAGENAME}Config.cmake")
set(NFITS_CONFIG_INSTALL_DIR   "cmake/${NFITS_PACKAGENAME}")
set(NFITS_TARGETS_EXPORT_NAME  "${NFITS_PACKAGENAME}Targets")

# Install NFITS lib
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT NFITS_Development)

install(
    TARGETS
        NFITS
    EXPORT ${NFITS_TARGETS_EXPORT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT NFITS_Runtime NAMELINK_COMPONENT NFITS_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT NFITS_Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT NFITS_Runtime
)

# If configured, install NFITSTests
if (BUILD_TESTING)
    install(
        TARGETS
            NFITSTests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT NFITS_Runtime
    )
endif()

write_basic_package_version_file(
    "${NFITS_VERSION_CONFIG_FILE}"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in" "${NFITS_PROJECT_CONFIG_FILE}" @ONLY)

install(
    FILES "${NFITS_VERSION_CONFIG_FILE}" "${NFITS_PROJECT_CONFIG_FILE}"
    DESTINATION "${NFITS_CONFIG_INSTALL_DIR}"
    COMPONENT NFITS_Development
)

install(
    EXPORT "${NFITS_TARGETS_EXPORT_NAME}"
    NAMESPACE "${NFITS_NAMESPACE}"
    DESTINATION "${NFITS_CONFIG_INSTALL_DIR}"
    COMPONENT NFITS_Development
)
]]

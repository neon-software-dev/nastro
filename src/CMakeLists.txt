cmake_minimum_required(VERSION 3.26.4)

project(NASTRO VERSION 0.0.1)

####
# Options
####

option(NASTRO_OPT_DEV_BUILD "Configure NASTRO for developer mode" OFF)

####
# Global variables
####

if (NASTRO_OPT_DEV_BUILD)
    message("NASTRO: Configuring for developer mode")

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(NASTRO_WARNINGS_FLAGS
            -Wall -Wextra -Werror -Wpedantic -Wcast-align -Wformat -Wformat=2 -Wlogical-op
            -Wmissing-include-dirs -Wpointer-arith -Wredundant-decls -Wsequence-point
            -Wshadow -Wswitch -Wunreachable-code -Wunused-but-set-parameter -Wfloat-equal
            -Wconversion -Wimplicit-fallthrough -Werror=format-security -Wtrampolines
            -Wbidi-chars=any
            -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
            -D_GLIBCXX_ASSERTIONS
            -fstrict-flex-arrays=3
            -fstack-clash-protection -fstack-protector-strong
            -Wl,-z,nodlopen -Wl,-z,noexecstack
            -Wl,-z,relro -Wl,-z,now
            -Wl,--as-needed -Wl,--no-copy-dt-needed-entries
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(NASTRO_WARNINGS_FLAGS
            -Wall -Wextra -Werror
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # no-op
    endif()
else()
    message("NASTRO: Configuring for non-developer mode")

    set(NASTRO_WARNINGS_FLAGS "")
endif()

# Flags for libraries to add to their compile options
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(NASTRO_LIBRARY_FLAGS
        -fvisibility=hidden
        -fvisibility-inlines-hidden
    )
endif()

####
# Projects
####

add_subdirectory(nfits)
add_subdirectory(nastroui)

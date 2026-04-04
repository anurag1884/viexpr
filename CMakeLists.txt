cmake_minimum_required(VERSION 3.20)
project(viexpr LANGUAGES CXX C)

# Set the C language standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate 'compile_commands.json' for use with the clangd language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32-bit
    set(_VCPKG_TARGET_TRIPLET_ARCH x86)
    if (WIN32)
        set(VCPKG_TARGET_TRIPLET x86-mingw-static CACHE STRING "" FORCE)
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET x86-osx CACHE STRING "" FORCE)
    elseif(UNIX AND NOT APPLE)
        set(VCPKG_TARGET_TRIPLET x86-linux CACHE STRING "" FORCE)
    endif()
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit
    set(_VCPKG_TARGET_TRIPLET_ARCH x64)
    if (WIN32)
        set(VCPKG_TARGET_TRIPLET x64-mingw-static CACHE STRING "" FORCE)
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET x64-osx CACHE STRING "" FORCE)
    elseif(UNIX AND NOT APPLE)
        set(VCPKG_TARGET_TRIPLET x64-linux CACHE STRING "" FORCE)
    endif()
endif()

# Bundle external libraries separately so compiler options don't affect them
add_library(ext_libs STATIC
    ${PROJECT_SOURCE_DIR}/ext/webview/src/webview.cc
    ${PROJECT_SOURCE_DIR}/ext/parson/src/parson.c
    ${PROJECT_SOURCE_DIR}/ext/tinyfiledialogs/src/tinyfiledialogs.c)
target_include_directories(ext_libs SYSTEM PUBLIC
    ${PROJECT_SOURCE_DIR}/ext/webview/include
    ${PROJECT_SOURCE_DIR}/ext/parson/include
    ${PROJECT_SOURCE_DIR}/ext/tinyfiledialogs/include)
if(WIN32)
    # Link MinGW dependencies statically (if present)
    if(MINGW)
        add_link_options("-static")
    endif()
    # Find the WebView2 framework
    find_package(unofficial-webview2 CONFIG REQUIRED)
    target_link_libraries(ext_libs PUBLIC unofficial::webview2::webview2)
    target_link_libraries(ext_libs PRIVATE winmm advapi32 ole32 shell32 shlwapi user32 version)
elseif(UNIX)
    if(APPLE)
        # Find the WebKit framework
        find_library(WEBKIT_FRAMEWORK WebKit)
        target_link_libraries(ext_libs PRIVATE ${WEBKIT_FRAMEWORK})
    else()
        find_package(PkgConfig REQUIRED)
        # Check for the modern webkit version first, fallback to older if needed
        pkg_check_modules(WEBVIEW_DEPS REQUIRED gtk+-3.0 webkit2gtk-4.1)
        # Add the include directories found by pkg-config
        target_include_directories(ext_libs PRIVATE ${WEBVIEW_DEPS_INCLUDE_DIRS})
        # Link the libraries and dl
        target_link_libraries(ext_libs PRIVATE ${WEBVIEW_DEPS_LIBRARIES} dl)
    endif()
endif()

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.c
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.cc
    ${PROJECT_SOURCE_DIR}/src/*.cxx
    ${PROJECT_SOURCE_DIR}/src/*.c++)
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SRCS})
target_compile_options(${PROJECT_NAME} PRIVATE
    -pedantic
    -pedantic-errors
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wshadow
    -Wno-deprecated-declarations
    -Wno-implicit-fallthrough
    # C++-specific options (ignored for C files)
    $<$<COMPILE_LANGUAGE:CXX>:-static-libstdc++>
    # C-specific options (ignored for C++ files)
    $<$<COMPILE_LANGUAGE:C>:-Wimplicit-function-declaration>
    $<$<COMPILE_LANGUAGE:C>:-Wimplicit-int>
    $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>)
target_link_libraries(${PROJECT_NAME} PRIVATE ext_libs)
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc $<$<COMPILE_LANGUAGE:CXX>:-static-libstdc++>)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

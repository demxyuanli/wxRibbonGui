cmake_minimum_required(VERSION 3.20)
project(wxapp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="D:/repos/vcpkg/scripts/buildsystems/vcpkg.cmake" -DwxWidgets_ROOT_DIR="D:/repos/vcpkg/installed/x64-windows"

# 设置 Vcpkg 工具链文件
set(CMAKE_TOOLCHAIN_FILE "D:/repos/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# 查找 wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base aui ribbon webview gl)
if(wxWidgets_FOUND)
    message(STATUS "wxWidgets found: ${wxWidgets_LIBRARIES}")
else()
    message(FATAL_ERROR "wxWidgets not found!")
endif()

find_package(PkgConfig REQUIRED)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBRSVG librsvg-2.0 IMPORTED_TARGET)
    if(LIBRSVG_FOUND)
        message(STATUS "Found librsvg-2.0, version ${LIBRSVG_VERSION}")
    else()
        message(WARNING "librsvg-2.0 not found. SVG rendering might be affected.")
    endif()
else()
    message(WARNING "PkgConfig not found. Cannot search for librsvg-2.0.")
endif()

# 查找其他依赖
find_package(NanoSVG CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(unofficial-theora CONFIG REQUIRED)

# 包含 wxWidgets 
include(${wxWidgets_USE_FILE})

# 添加子目录
add_subdirectory(src)
add_subdirectory(src/logger)
add_subdirectory(src/config)
add_subdirectory(src/language)
add_subdirectory(src/flatui)

# 收集源文件
set(SOURCES
    ${MAINAPPLICATION_SOURCES}
    ${LOGGER_SOURCES}
    ${CONFIG_SOURCES}
    ${LANGUAGE_SOURCES}
    ${FLATUI_SOURCES}
)

# 调试源文件
message(STATUS "LOGGER_SOURCES: ${LOGGER_SOURCES}")
message(STATUS "All SOURCES: ${SOURCES}")

# 创建可执行文件
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# 在 Visual Studio 中按目录结构组织文件
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SOURCES})

# 设置头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${wxWidgets_INCLUDE_DIRS}
    ${JsonCpp_INCLUDE_DIRS}
)

if(LIBRSVG_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::LIBRSVG)
    # You might also need to include directories if headers are not found automatically
    # target_include_directories(wxgui_executable PRIVATE ${LIBRSVG_INCLUDE_DIRS})
    # And add compile definitions if needed
    # target_compile_definitions(wxgui_executable PRIVATE ${LIBRSVG_CFLAGS_OTHER})
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${wxWidgets_LIBRARIES}
    utf8::cpp
    jsoncpp_lib
    unofficial::theora::theora
    unofficial::theora::theoradec
    unofficial::theora::theoraenc
)

# Windows 下启用 Unicode 支持
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# MSVC 特定设置
if(MSVC)
    # 强制在 Debug 模式下使用 /ZI 支持“编辑并继续”
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI /Od>)
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )

    # 复制 config 目录及其所有子目录和文件到目标目录
    set(CONFIG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/config")
    set(TARGET_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    
    # 使用 copy_directory 命令复制整个目录
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CONFIG_SOURCE_DIR}"
                "${TARGET_DIR}/config"
        COMMENT "Copying config directory and all subdirectories to ${TARGET_DIR}/config"
    )
endif()

# 调试依赖路径
message(STATUS "Project include dir: ${CMAKE_SOURCE_DIR}/include")
cmake_minimum_required(VERSION 3.21)
project(Screenshot LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 官方预编译库使用 /MD；与动态 Qt 链接时需与 CRT 一致
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 静态 MFC（截图层仍用 MFC 对话框）
set(CMAKE_MFC_FLAG 2)

set(SCREENSHOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Screenshot)

set(SCREENSHOT_SOURCES
  ${SCREENSHOT_DIR}/Screenshot.cpp
  ${SCREENSHOT_DIR}/AppSettings.cpp
  ${SCREENSHOT_DIR}/AnnotationLayer.cpp
  ${SCREENSHOT_DIR}/CatchScreenDlg.cpp
  ${SCREENSHOT_DIR}/DarkToolBar.cpp
  ${SCREENSHOT_DIR}/MyEdit.cpp
  ${SCREENSHOT_DIR}/MyTracker.cpp
  ${SCREENSHOT_DIR}/ScreenCapture.cpp
  ${SCREENSHOT_DIR}/StartupUtil.cpp
  ${SCREENSHOT_DIR}/TextAnnotOverlay.cpp
  ${SCREENSHOT_DIR}/stdafx.cpp
  ${SCREENSHOT_DIR}/qt/MainWindow.cpp
  ${SCREENSHOT_DIR}/qt/SettingsDialog.cpp
  ${SCREENSHOT_DIR}/qt/ScreenshotBridge.cpp
  ${SCREENSHOT_DIR}/qt/QtMfcMain.cpp
  ${SCREENSHOT_DIR}/Screenshot.rc
)

add_executable(Screenshot WIN32 ${SCREENSHOT_SOURCES})

target_include_directories(Screenshot PRIVATE
  ${SCREENSHOT_DIR}
  ${SCREENSHOT_DIR}/qt
)

target_compile_definitions(Screenshot PRIVATE
  UNICODE
  _UNICODE
  _WINDOWS
  NOMINMAX
)

target_compile_options(Screenshot PRIVATE /utf-8)

target_link_libraries(Screenshot PRIVATE
  Qt6::Widgets
  Gdiplus
  Msimg32
  Shcore
  Shell32
  Ole32
)

if(MSVC)
  target_link_options(Screenshot PRIVATE
    "/MANIFESTUAC:level='asInvoker' uiAccess='false'"
  )
  if(EXISTS ${SCREENSHOT_DIR}/app.manifest)
    set_target_properties(Screenshot PROPERTIES
      VS_DPI_AWARE "PerMonitor"
    )
  endif()
endif()

install(TARGETS Screenshot RUNTIME DESTINATION .)

# 本地 / CI：cmake --install build --prefix dist 后执行 windeployqt
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin" "${Qt6_DIR}/../../bin")
if(WINDEPLOYQT_EXECUTABLE)
  add_custom_command(TARGET Screenshot POST_BUILD
    COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:Screenshot>" --no-translations
    COMMENT "Running windeployqt for portable Qt runtime"
  )
endif()

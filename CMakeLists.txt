cmake_minimum_required (VERSION 3.14)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ColonyFinder")

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.90.5
)
FetchContent_MakeAvailable(imgui)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAPS ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_TELNET ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_DICT ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_FILE ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_TFTP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_GOPHER ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_SMTP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_POP3 ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_IMAP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_RTSP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_RTMP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_MQTT ON CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)

if (WIN32)
  set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
  set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
else()
  find_package(OpenSSL REQUIRED)
  set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
  curl
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG curl-8_4_0
)
FetchContent_MakeAvailable(curl)

FetchContent_Declare(
  zlib
  URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip
)
FetchContent_MakeAvailable(zlib)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable (ColonyFinder
    "src/ColonyFinder.cpp"
    "src/ColonyFinder.h"
    "src/download/DownloadSubject.cpp"
    "src/download/DownloadSubject.h"
    "src/download/HttpDownloader.cpp"
    "src/download/HttpDownloader.h"
    "src/download/GzipDecompressor.cpp"
    "src/download/GzipDecompressor.h"
    "src/download/JsonStreamValidator.cpp"
    "src/download/JsonStreamValidator.h"
    "src/ui/AppController.cpp"
    "src/ui/AppController.h"
    "src/ui/AppTheme.h"
    "src/ui/FolderDialog.cpp"
    "src/ui/FolderDialog.h"
    "src/ui/SettingsObserver.h"
    "src/ui/SettingsService.cpp"
    "src/ui/SettingsService.h"
    "src/ui/states/IAppState.h"
    "src/ui/states/WelcomeState.cpp"
    "src/ui/states/WelcomeState.h"
    "src/ui/states/SelectDownloadState.cpp"
    "src/ui/states/SelectDownloadState.h"
    "src/ui/states/DownloadingState.cpp"
    "src/ui/states/DownloadingState.h"
    "src/ui/states/ExtractionState.cpp"
    "src/ui/states/ExtractionState.h"
    "src/ui/states/SchemaUpdateState.cpp"
    "src/ui/states/SchemaUpdateState.h"
    "src/ui/states/MainAppState.cpp"
    "src/ui/states/MainAppState.h"
    "src/ui/states/ErrorState.cpp"
    "src/ui/states/ErrorState.h"
)

target_include_directories(ColonyFinder PRIVATE src)

if (WIN32)
  target_link_libraries(ColonyFinder PRIVATE imgui libcurl zlibstatic nlohmann_json ole32 uuid)
else()
  target_link_libraries(ColonyFinder PRIVATE imgui libcurl zlib nlohmann_json)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ColonyFinder PROPERTY CXX_STANDARD 20)
endif()

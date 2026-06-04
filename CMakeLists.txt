cmake_minimum_required (VERSION 3.14)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ColonyFinder" VERSION 1.0.4)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h
    @ONLY
)

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

set(ZLIB_COMPAT ON CACHE BOOL "" FORCE)
set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ZLIBNG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_GTEST OFF CACHE BOOL "" FORCE)
set(WITH_OPTIM ON CACHE BOOL "" FORCE)
set(WITH_NEW_STRATEGIES ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
  GIT_TAG 2.2.1
)
FetchContent_MakeAvailable(zlib)

FetchContent_Declare(
  curl
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG curl-8_4_0
)
FetchContent_MakeAvailable(curl)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

set(SIMDJSON_JUST_LIBRARY ON CACHE BOOL "" FORCE)
set(SIMDJSON_BUILD_STATIC_LIB ON CACHE BOOL "" FORCE)
set(SIMDJSON_DEVELOPER_MODE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG v3.10.1
)
FetchContent_MakeAvailable(simdjson)

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
    "src/download/DynamicSaxValidator.h"
    "src/download/PipeStreamBuf.cpp"
    "src/download/PipeStreamBuf.h"
    "src/download/StreamingProcessor.cpp"
    "src/download/StreamingProcessor.h"
    "src/download/DatabaseService.cpp"
    "src/download/DatabaseService.h"
    "src/download/DatabaseObserver.h"
    "src/search/CompositeJsonSax.h"
    "src/search/SimdjsonIndexer.cpp"
    "src/search/SimdjsonIndexer.h"
    "src/download/LineSink.cpp"
    "src/download/LineSink.h"
    "src/search/SystemIndex.h"
    "src/search/IndexProgressStreamBuf.h"
    "src/search/IndexBuilderSax.cpp"
    "src/search/IndexBuilderSax.h"
    "src/search/IndexBuilder.cpp"
    "src/search/IndexBuilder.h"
    "src/search/SearchEngine.cpp"
    "src/search/SearchEngine.h"
    "src/search/SearchObserver.h"
    "src/search/AsyncSearchService.cpp"
    "src/search/AsyncSearchService.h"
    "src/ui/WindowManager.cpp"
    "src/ui/WindowManager.h"
    "src/ui/AppController.cpp"
    "src/ui/AppController.h"
    "src/ui/AppTheme.h"
    "src/ui/FolderDialog.cpp"
    "src/ui/FolderDialog.h"
    "src/ui/SettingsObserver.h"
    "src/ui/SettingsService.cpp"
    "src/ui/SettingsService.h"
    "src/ui/SystemMemory.cpp"
    "src/ui/SystemMemory.h"
    "src/ui/UiUtils.cpp"
    "src/ui/UiUtils.h"
    "src/ui/components/FilterPanel.cpp"
    "src/ui/components/FilterPanel.h"
    "src/ui/components/ResultsPanel.cpp"
    "src/ui/components/ResultsPanel.h"
    "src/ui/states/IAppState.h"
    "src/ui/states/WelcomeState.cpp"
    "src/ui/states/WelcomeState.h"
    "src/ui/states/SelectDownloadState.cpp"
    "src/ui/states/SelectDownloadState.h"
    "src/ui/states/DownloadIndexState.cpp"
    "src/ui/states/DownloadIndexState.h"
    "src/ui/states/DownloadingIndexState.cpp"
    "src/ui/states/DownloadingIndexState.h"
    "src/ui/states/DownloadingState.cpp"
    "src/ui/states/DownloadingState.h"
    "src/ui/states/ExtractionState.cpp"
    "src/ui/states/ExtractionState.h"
    "src/ui/states/IndexOutdatedState.cpp"
    "src/ui/states/IndexOutdatedState.h"
    "src/ui/states/SchemaUpdateState.cpp"
    "src/ui/states/SchemaUpdateState.h"
    "src/ui/states/MainAppState.cpp"
    "src/ui/states/MainAppState.h"
    "src/ui/states/ErrorState.cpp"
    "src/ui/states/ErrorState.h"
    "src/ui/states/UpdateAvailableState.cpp"
    "src/ui/states/UpdateAvailableState.h"
    "src/ui/states/SettingsState.cpp"
    "src/ui/states/SettingsState.h"
    "src/update/UpdateObserver.h"
    "src/update/UpdateSubject.cpp"
    "src/update/UpdateSubject.h"
    "src/update/IUpdateChecker.h"
    "src/update/GithubUpdateChecker.cpp"
    "src/update/GithubUpdateChecker.h"
    "src/update/UpdateService.cpp"
    "src/update/UpdateService.h"
    "src/update/UpdaterLauncher.cpp"
    "src/update/UpdaterLauncher.h"
)

target_include_directories(ColonyFinder PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/generated)

if (WIN32)
  target_link_libraries(ColonyFinder PRIVATE imgui libcurl zlibstatic nlohmann_json simdjson ole32 uuid)
else()
  target_link_libraries(ColonyFinder PRIVATE imgui libcurl zlib nlohmann_json simdjson)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ColonyFinder PROPERTY CXX_STANDARD 20)
endif()

add_executable(colony_updater
    "src/updater/main.cpp"
    "src/logging/LogLevel.h"
    "src/logging/LogLevel.cpp"
    "src/logging/LogRecord.h"
    "src/logging/LogRecord.cpp"
    "src/logging/ILogSink.h"
    "src/logging/ConsoleLogSink.h"
    "src/logging/ConsoleLogSink.cpp"
    "src/logging/FileLogSink.h"
    "src/logging/FileLogSink.cpp"
    "src/logging/Logger.h"
    "src/logging/Logger.cpp"
)
set_property(TARGET colony_updater PROPERTY CXX_STANDARD 20)
target_include_directories(colony_updater PRIVATE src)
if (WIN32)
  set_target_properties(colony_updater PROPERTIES WIN32_EXECUTABLE FALSE)
endif()

add_executable(colony_indexer
    "src/server_indexer_tool/main.cpp"
    "src/server_indexer_tool/ArgumentParser.cpp"
    "src/server_indexer_tool/ArgumentParser.h"
    "src/server_indexer_tool/CliSelfUpdater.cpp"
    "src/server_indexer_tool/CliSelfUpdater.h"
    "src/server_indexer_tool/CliAppController.cpp"
    "src/server_indexer_tool/CliAppController.h"
    "src/download/DownloadSubject.cpp"
    "src/download/DownloadSubject.h"
    "src/download/HttpDownloader.cpp"
    "src/download/HttpDownloader.h"
    "src/download/GzipDecompressor.cpp"
    "src/download/GzipDecompressor.h"
    "src/download/JsonStreamValidator.cpp"
    "src/download/JsonStreamValidator.h"
    "src/download/DynamicSaxValidator.h"
    "src/download/PipeStreamBuf.cpp"
    "src/download/PipeStreamBuf.h"
    "src/download/StreamingProcessor.cpp"
    "src/download/StreamingProcessor.h"
    "src/download/DatabaseService.cpp"
    "src/download/DatabaseService.h"
    "src/download/DatabaseObserver.h"
    "src/download/DownloadObserver.h"
    "src/search/CompositeJsonSax.h"
    "src/search/SimdjsonIndexer.cpp"
    "src/search/SimdjsonIndexer.h"
    "src/download/LineSink.cpp"
    "src/download/LineSink.h"
    "src/search/SystemIndex.h"
    "src/search/IndexProgressStreamBuf.h"
    "src/search/IndexBuilderSax.cpp"
    "src/search/IndexBuilderSax.h"
    "src/search/IndexBuilder.cpp"
    "src/search/IndexBuilder.h"
    "src/ui/SettingsService.cpp"
    "src/ui/SettingsService.h"
    "src/ui/SettingsObserver.h"
    "src/update/IUpdateChecker.h"
    "src/update/GithubUpdateChecker.cpp"
    "src/update/GithubUpdateChecker.h"
    "src/update/UpdaterLauncher.cpp"
    "src/update/UpdaterLauncher.h"
)
set_property(TARGET colony_indexer PROPERTY CXX_STANDARD 20)
target_include_directories(colony_indexer PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/generated)
if (WIN32)
  target_link_libraries(colony_indexer PRIVATE libcurl zlibstatic nlohmann_json simdjson ole32 uuid)
else()
  target_link_libraries(colony_indexer PRIVATE libcurl zlib nlohmann_json simdjson)
endif()

add_custom_command(TARGET ColonyFinder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:colony_updater>
        $<TARGET_FILE_DIR:ColonyFinder>/$<TARGET_FILE_NAME:colony_updater>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:colony_indexer>
        $<TARGET_FILE_DIR:ColonyFinder>/$<TARGET_FILE_NAME:colony_indexer>
)


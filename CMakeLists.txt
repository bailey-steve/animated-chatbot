cmake_minimum_required(VERSION 3.20)
project(Chatbot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Quick
    3DCore
    3DRender
    3DExtras
    Multimedia
)

# Use FetchContent for dependencies
include(FetchContent)

# nlohmann/json for JSON parsing
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# cpr for HTTP requests (Ollama API)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
)
set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpr)

# spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Source files
set(SOURCES
    main.cpp
    # Core
    src/core/Application.cpp
    # Chat
    src/chat/ChatEngine.cpp
    src/chat/ConversationHistory.cpp
    # TTS
    src/tts/TTSEngine.cpp
    src/tts/PhonemeExtractor.cpp
    # UI
    src/ui/MainWindow.cpp
)

set(HEADERS
    # Core
    src/core/Application.h
    src/core/EventBus.h
    # Chat
    src/chat/ChatEngine.h
    src/chat/ConversationHistory.h
    # TTS
    src/tts/TTSEngine.h
    src/tts/PhonemeExtractor.h
    # UI
    src/ui/MainWindow.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DExtras
    Qt6::Multimedia
    nlohmann_json::nlohmann_json
    cpr::cpr
    spdlog::spdlog
)

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION ${CMAKE_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/config
    DESTINATION ${CMAKE_BINARY_DIR}
)

# Copy Piper TTS binaries and voices to build directory
file(COPY ${CMAKE_SOURCE_DIR}/third_party/piper
    DESTINATION ${CMAKE_BINARY_DIR}/third_party
)
file(COPY ${CMAKE_SOURCE_DIR}/third_party/voices
    DESTINATION ${CMAKE_BINARY_DIR}/third_party
)

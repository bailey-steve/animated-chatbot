# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an **Animated 3D Chatbot** application - a C++ desktop app with local LLM integration (Ollama), designed to eventually feature 3D animated avatars with lip-sync, emotional expressions, and multiple personalities. Currently in **Phase 1** (foundation complete), with 7 phases total planned.

**Current Status**: Phase 1 complete - Qt6 UI with working Ollama chat integration
**Technology**: C++20, Qt6, CMake 3.20+

## Build & Run Commands

### Initial Setup
```bash
# Configure CMake (takes ~3 minutes - downloads dependencies)
mkdir -p build && cd build && cmake ..

# Build (from build/ directory)
make -j$(nproc)

# Run (from build/ directory)
./Chatbot
```

### Incremental Development
```bash
# After code changes, rebuild from build/ directory
make -j$(nproc)

# Quick rebuild of specific file (example)
make -j$(nproc) Chatbot
```

### Prerequisites Check
```bash
# Verify Ollama is running (required for chat functionality)
ps aux | grep ollama
ollama serve &  # Start if not running

# Verify model is available
ollama list  # Should show llama3.2:3b
```

### Clean Build
```bash
# From project root
rm -rf build
mkdir -p build && cd build && cmake .. && make -j$(nproc)
```

## Architecture & Key Concepts

### Component Structure

The application follows a **component-based architecture** with Qt signal/slot communication:

```
Application (singleton coordinator)
├── ChatEngine (Ollama API client, async processing)
│   └── ConversationHistory (message history, max 100 messages)
└── MainWindow (Qt6 UI, chat display)
```

**Critical architectural points**:

1. **All classes are in the `Chatbot` namespace** - Forward declarations must be inside the namespace
2. **Singleton pattern**: `Application` has a static `s_instance` for global access
3. **Qt Signal/Slot communication**: Components don't call each other directly; they emit signals
4. **Async LLM processing**: ChatEngine uses `QtConcurrent::run()` to avoid blocking UI

### Signal Flow

```
User Input → MainWindow::userMessageSubmitted
          → ChatEngine::sendMessage
          → (async) callOllamaAPI
          → ChatEngine::responseReceived
          → MainWindow::addBotMessage
```

### Ollama API Integration

**Important**: The Ollama `/api/generate` endpoint expects:
- `prompt`: String (current + conversation history embedded)
- `model`: String (e.g., "llama3.2:3b")
- `system`: String (system prompt)
- `stream`: false (we use non-streaming mode)

**Do NOT use the `context` field** - it expects an array of token IDs (integers), not a conversation string. Instead, embed conversation history into the `prompt` field as formatted text.

Current implementation in `ChatEngine::callOllamaAPI()`:
```cpp
// Build conversation history into prompt
std::string fullPrompt;
if (messages.size() > 1) {
    fullPrompt += "Previous conversation:\n";
    // Include previous messages...
    fullPrompt += "\nCurrent question:\n";
}
fullPrompt += prompt;
```

### Qt Specifics

**MOC (Meta-Object Compiler)**: CMake has `CMAKE_AUTOMOC ON`, so Qt preprocesses files with `Q_OBJECT` macro automatically. If you add/modify signals/slots, rebuild will trigger MOC.

**Qt Concurrent**: We use `QtConcurrent::run()` with `QFutureWatcher` for async operations. The watcher's `finished()` signal brings results back to the main thread.

## File Organization

```
src/
├── core/           # Application lifecycle, event bus
├── chat/           # LLM integration, conversation management
├── ui/             # Qt widgets and windows
├── tts/            # (Phase 2) Text-to-speech - NOT YET IMPLEMENTED
├── avatar/         # (Phase 3) 3D rendering - NOT YET IMPLEMENTED
├── emotion/        # (Phase 5) Sentiment analysis - NOT YET IMPLEMENTED
└── personality/    # (Phase 6) Personality configs - NOT YET IMPLEMENTED
```

**When adding new source files**: Update `CMakeLists.txt` in both `SOURCES` and `HEADERS` sections. CMake will not auto-detect new files.

## Adding New Components

### 1. Create files in appropriate directory
```cpp
// Example: src/tts/TTSEngine.h
namespace Chatbot {
class TTSEngine : public QObject {
    Q_OBJECT
    // ...
};
}
```

### 2. Update CMakeLists.txt
```cmake
set(SOURCES
    # ... existing sources ...
    src/tts/TTSEngine.cpp
)

set(HEADERS
    # ... existing headers ...
    src/tts/TTSEngine.h
)
```

### 3. Wire into Application
In `Application::initializeComponents()`:
```cpp
m_ttsEngine = std::make_unique<TTSEngine>();
```

In `Application::setupConnections()`:
```cpp
QObject::connect(m_chatEngine.get(), &ChatEngine::responseReceived,
                m_ttsEngine.get(), &TTSEngine::speak);
```

## Common Patterns

### Adding UI Colors/Styles
All message styling is in `MainWindow::addUserMessage()`, `addBotMessage()`, and `addSystemMessage()`. Use inline HTML with explicit colors:
```cpp
QString html = QString(
    "<span style='color: #000;'>%1</span>"  // Always specify color explicitly
).arg(text);
```

**Background colors**: Set in `setupUI()` via `setStyleSheet()` on widgets. Current chat background is `#ffffff` (white) with `color: #000000` (black text).

### Configuration Changes

**Ollama settings** are hardcoded in `ChatEngine` constructor:
```cpp
m_ollamaUrl = "http://localhost:11434"
m_model = "llama3.2:3b"
m_systemPrompt = "You are a helpful, friendly assistant."
```

To use a different model: Edit these defaults or add setter calls in `Application::initializeComponents()`.

## Dependencies

All dependencies are **auto-downloaded via CMake FetchContent**:
- **nlohmann/json** v3.11.3 - JSON parsing
- **cpr** v1.10.5 - HTTP client (wraps libcurl)
- **spdlog** v1.12.0 - Structured logging

**Qt6 components** (must be system-installed):
- Core, Widgets, Quick (QML)
- 3DCore, 3DRender, 3DExtras (for future Phase 3)
- Multimedia (for future Phase 2)

## Debugging

### Logging
Uses `spdlog`. Current log level is DEBUG (set in `Application` constructor):
```cpp
spdlog::set_level(spdlog::level::debug);
```

Logs go to stdout. Example:
```
[2025-12-06 12:06:28.187] [debug] Sending request to Ollama: http://localhost:11434/api/generate
```

### Common Runtime Issues

**"Empty response from Ollama API"**:
- Check Ollama is running: `ps aux | grep ollama`
- Check HTTP 400 errors in logs - usually means API request format is wrong
- Verify model exists: `ollama list`

**White text on white background**:
- Fixed in MainWindow.cpp. All text now has explicit `color: #000` or similar.

**Namespace compilation errors**:
- Ensure forward declarations are **inside** `namespace Chatbot { }`, not before it
- Check all classes use `Chatbot::ClassName` in includes

## Multi-Phase Development

This project is being built in phases. **Do not implement future phase features preemptively**:

- **Phase 1** ✅: Qt UI, Ollama chat (COMPLETE)
- **Phase 2** 🔄: Piper TTS integration (NEXT)
- **Phase 3-7**: Avatar, lip-sync, emotions, personalities, polish

When implementing Phase 2+:
1. Create new subdirectory in `src/` (e.g., `src/tts/`)
2. Add component class (e.g., `TTSEngine`)
3. Update CMakeLists.txt
4. Wire signals in `Application::setupConnections()`
5. Update this CLAUDE.md with new architectural details

## Testing

**Currently manual testing only**. Automated tests planned for Phase 7.

Manual test procedure (from README.md):
1. UI loads without crashes
2. Type message, click Send → appears in chat
3. Bot responds within 5 seconds
4. Multi-turn conversation maintains context

To test after changes:
```bash
cd build
make -j$(nproc)
./Chatbot
# Type "My name is Alex" then "What is my name?" to test context
```

## Development Workflow

1. **Make changes** to .cpp/.h files
2. **Rebuild**: `cd build && make -j$(nproc)`
3. **Test**: Run `./Chatbot` and interact with UI
4. **Check logs**: Look for spdlog output for errors
5. If Ollama errors occur, check JSON request format with `spdlog::debug()`

**Hot tip**: Keep Ollama running in background during development to avoid startup delays.

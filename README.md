# Animated 3D Chatbot

A C++ desktop chatbot application featuring local LLM integration (Ollama), 3D animated avatars with accurate lip-sync, emotional expressions, and multiple selectable personalities.

## Features

### Current
#### Phase 1 - ✅ Completed
- ✅ Qt6-based modern UI with chat interface
- ✅ Local LLM integration via Ollama API
- ✅ Conversation history management
- ✅ Asynchronous message processing
- ✅ Error handling and logging

#### Phase 2 - ✅ Completed
- ✅ Text-to-Speech with Piper TTS
- ✅ Phoneme extraction pipeline
- ✅ Audio playback with Qt Multimedia
- ✅ Automatic speech synthesis for bot responses
- ✅ High-quality neural TTS voice (Lessac)

### Planned
- 🔄 **Phase 3**: 3D avatar display (Qt3D/OpenGL with GLTF models)
- 🔄 **Phase 4**: Accurate lip-sync system (phoneme → viseme mapping)
- 🔄 **Phase 5**: Emotional expressions (sentiment analysis)
- 🔄 **Phase 6**: Multiple personalities (JSON-based configs)
- 🔄 **Phase 7**: Polish & optimization

## Prerequisites

### System Requirements
- **OS**: Linux (Ubuntu 20.04+), macOS, or Windows
- **CPU**: Modern x86_64 processor
- **RAM**: 8GB minimum (16GB recommended for LLM)
- **Storage**: 5GB for models and dependencies

### Required Software

1. **Qt6** (6.9.2 or later)
   ```bash
   sudo apt update
   sudo apt install qt6-base-dev qt6-multimedia-dev qt6-3d-dev \
                    qt6-declarative-dev libgl1-mesa-dev
   ```

2. **Build Tools**
   ```bash
   sudo apt install build-essential cmake git
   ```

3. **Ollama** (for local LLM)
   ```bash
   curl -fsSL https://ollama.com/install.sh | sh
   ```

4. **Download LLM Model**
   ```bash
   ollama pull llama3.2:3b
   ```

5. **Piper TTS** (for text-to-speech)
   ```bash
   # Download Piper TTS binary
   mkdir -p third_party && cd third_party
   wget https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_linux_x86_64.tar.gz
   tar -xzf piper_linux_x86_64.tar.gz

   # Download voice model
   mkdir -p voices && cd voices
   wget https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/medium/en_US-lessac-medium.onnx
   wget https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/medium/en_US-lessac-medium.onnx.json
   cd ../..
   ```

## Building the Project

### 1. Clone or Navigate to Project
```bash
cd /home/sxbailey/CLionProjects/Chatbot
```

### 2. Configure with CMake
```bash
mkdir -p build
cd build
cmake ..
```

This will:
- Find Qt6 components
- Download and build dependencies (nlohmann/json, cpr, spdlog)
- Configure the build system (takes ~3 minutes)

### 3. Compile
```bash
make -j$(nproc)
```

Build output: `Chatbot` executable (~423KB)

### 4. Run
```bash
./Chatbot
```

## Testing

### Manual Testing Strategy

#### 1. **UI Functionality Test**
**Objective**: Verify Qt window displays and UI is responsive

**Steps**:
1. Run the application: `./Chatbot`
2. **Expected**: Qt window opens with title "Chatbot - Animated 3D Assistant"
3. **Verify**:
   - Chat display area (gray background)
   - Input field with placeholder text
   - Green "Send" button
   - Window size 800x600, centered on screen

**Pass Criteria**: Window displays without crashes, UI elements are visible

---

#### 2. **Chat Input Test**
**Objective**: Test user input handling

**Steps**:
1. Type "Hello" in the input field
2. Click "Send" button (or press Enter)
3. **Expected**:
   - Message appears in chat display with timestamp
   - Formatted as "You: Hello"
   - Input field clears
   - System message "Thinking..." appears

**Pass Criteria**: User messages display correctly, input clears after send

---

#### 3. **Ollama Integration Test**
**Objective**: Verify LLM communication works

**Prerequisites**:
- Ollama service running: `ollama serve &`
- Model downloaded: `ollama list` shows llama3.2:3b

**Steps**:
1. Ensure Ollama is running (check with `ps aux | grep ollama`)
2. In chat, send message: "What is 2+2?"
3. **Expected**:
   - "System: Thinking..." appears
   - After 1-3 seconds, bot response appears
   - Formatted as "Bot: [response text]"
   - Response should answer "4" or similar

**Pass Criteria**:
- Bot responds within 5 seconds
- Response is coherent and relevant
- No error messages displayed

---

#### 4. **Error Handling Test**
**Objective**: Test behavior when Ollama is unavailable

**Steps**:
1. Stop Ollama: `killall ollama` (if running)
2. Send a message in the chat
3. **Expected**:
   - System error message appears
   - Application doesn't crash
   - User can still type messages

**Restart Ollama**: `ollama serve &`

**Pass Criteria**: Graceful error handling, no crashes

---

#### 5. **Conversation History Test**
**Objective**: Verify multi-turn conversations work

**Steps**:
1. Send: "My name is Alex"
2. Wait for response
3. Send: "What is my name?"
4. **Expected**: Bot remembers and responds with "Alex"

**Pass Criteria**: Context is maintained across messages

---

### Automated Testing (Future)
Planned for Phase 7:
- Unit tests using Google Test
- Integration tests for component interactions
- Performance tests (FPS, latency measurements)

## Project Structure

```
Chatbot/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── .gitignore                  # Git ignore rules
├── main.cpp                    # Application entry point
├── src/
│   ├── core/
│   │   ├── Application.{h,cpp} # Main app coordinator
│   │   └── EventBus.h          # Signal/slot communication
│   ├── chat/
│   │   ├── ChatEngine.{h,cpp}  # LLM integration (Ollama)
│   │   └── ConversationHistory.{h,cpp}
│   ├── ui/
│   │   └── MainWindow.{h,cpp}  # Qt chat interface
│   ├── tts/
│   │   ├── TTSEngine.{h,cpp}   # Piper TTS integration
│   │   └── PhonemeExtractor.{h,cpp} # Phoneme parsing
│   ├── avatar/                 # (Phase 3) 3D rendering
│   ├── emotion/                # (Phase 5) Sentiment analysis
│   └── personality/            # (Phase 6) Personality configs
├── config/                     # Configuration files
│   └── personalities/          # Personality definitions (future)
├── assets/                     # Asset files
│   ├── models/                 # 3D models (future)
│   └── voices/                 # TTS voice models (future)
└── build/                      # Build artifacts (gitignored)
```

## Architecture

### Component Diagram
```
┌─────────────────────────────┐
│   Qt6 Main Application      │
│       (MainWindow)           │
└────────┬────────────────────┘
         │
    ┌────┴────┐
    │  Core   │
    │  App    │
    └────┬────┘
         │
    ┌────┴─────────────────┐
    ▼         ▼            ▼
┌────────┐┌────────┐  ┌──────────┐
│  Chat  ││  TTS   │  │   UI     │
│ Engine ││ Engine │  │ Main     │
│(Ollama)││(Piper) │  │ Window   │
└────┬───┘└───┬────┘  └──────────┘
     │        │
┌────▼────┐  │
│Conver-  │  │
│sation   │  │
│History  │  │
└─────────┘  ▼
         ┌────────────┐
         │  Phoneme   │
         │ Extractor  │
         └────────────┘
```

### Technology Stack
- **GUI**: Qt6 (Widgets, Quick, 3D, Multimedia)
- **LLM**: Ollama API → llama.cpp (future migration)
- **TTS**: Piper TTS (Phase 2)
- **3D**: Qt3D with GLTF 2.0 models (Phase 3)
- **JSON**: nlohmann/json
- **HTTP**: cpr (libcpr)
- **Logging**: spdlog
- **Build**: CMake 3.20+, C++20

## Configuration

### Changing Ollama Settings
Edit `src/chat/ChatEngine.cpp`:
```cpp
// Default settings (constructor):
m_ollamaUrl = "http://localhost:11434"  // Ollama server URL
m_model = "llama3.2:3b"                 // Model name
m_systemPrompt = "You are a helpful..."  // System prompt
```

### Using Different Models
```bash
# List available models
ollama list

# Pull a different model
ollama pull mistral:7b

# Update ChatEngine.cpp to use new model
m_model = "mistral:7b"
```

## Troubleshooting

### Build Issues

**Q**: CMake can't find Qt6
**A**: Ensure Qt6 is installed and in PATH. Try: `export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6`

**Q**: Compilation errors about forward declarations
**A**: This was fixed in commit. Ensure you have the latest code.

### Runtime Issues

**Q**: Window doesn't appear
**A**: Check if Qt6 libraries are installed: `ldd ./Chatbot | grep Qt6`

**Q**: "Failed to connect to Ollama"
**A**:
1. Check Ollama is running: `ps aux | grep ollama`
2. Start Ollama: `ollama serve &`
3. Verify model is downloaded: `ollama list`

**Q**: Slow responses
**A**:
- Use a smaller model (llama3.2:1b instead of 3b)
- Increase system RAM
- Check CPU usage during inference

## Development Roadmap

### ✅ Phase 1: Foundation (Weeks 1-4) - COMPLETED
- Basic Qt UI
- Ollama integration
- Chat functionality

### ✅ Phase 2: TTS (Weeks 5-6) - COMPLETED
**Implemented**:
- ✅ Integrated Piper TTS (subprocess approach)
- ✅ Phoneme extraction with timing data
- ✅ Qt Multimedia audio playback
- ✅ Automatic speech synthesis for bot responses
- ✅ High-quality neural TTS voice (Lessac)

**Testing**:
- ✅ TTS generates audio successfully
- ✅ Phoneme timing extracted accurately
- ✅ Audio playback synchronized

### 🔄 Phase 3: 3D Avatar (Weeks 7-9) - NEXT
**Goals**:
- Qt3D scene setup
- Load GLTF models
- Basic idle animation

**Testing**:
- Avatar displays correctly
- 60 FPS rendering
- Model loading performance

### 🔄 Phase 4: Lip-Sync (Weeks 10-12) - CRITICAL
**Goals**:
- Phoneme → viseme mapping
- Blend shape animation
- Audio-visual sync

**Testing**:
- Lip movements match audio
- Sync drift < 50ms
- Smooth transitions

### 🔄 Phase 5: Emotions (Weeks 13-14)
**Goals**:
- Sentiment analysis
- Facial expressions
- Emotion blending

**Testing**:
- Expressions match sentiment
- No conflict with lip-sync
- Emotion detection accuracy

### 🔄 Phase 6: Personalities (Weeks 15-16)
**Goals**:
- JSON personality configs
- Multiple characters
- Voice/model switching

**Testing**:
- Personality changes work
- Distinct behaviors
- Smooth transitions

### 🔄 Phase 7: Polish (Weeks 17-18)
**Goals**:
- Performance optimization
- UI/UX improvements
- Bug fixes

**Testing**:
- Performance profiling
- Memory leak detection
- User acceptance testing

## Contributing

This is currently a personal project. Future contributions may be welcome after Phase 7.

## License

To be determined.

## Acknowledgments

- **Qt Framework**: Cross-platform UI
- **Ollama**: Local LLM inference
- **Piper TTS**: Neural text-to-speech synthesis
- **Anthropic Claude**: Development assistance
- **Open Source Libraries**: nlohmann/json, cpr, spdlog

---

**Version**: 2.0.0 (Phase 2)
**Last Updated**: 2025-12-06
**Status**: Active Development

# Chess-vs-AI

## About
A high performing browser-based chess application with a C++ engine compiled to WebAssembly\
Engine strength: ~2500 ELO (Estimated)

## Requirements
To build and run this project, the following are required:
- A modern browser with WebAssembly support
- [Node.js](https://nodejs.org/)
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- [CMake](https://cmake.org/)
- [Ninja](https://github.com/ninja-build/ninja)

## Setup
### 1. Clone the repository
```bash
git clone https://github.com/salmancheema155/Chess-vs-AI.git
```

### 2. Build the C++ engine
Emscripten must be installed and activated\
Installation guide: https://emscripten.org/docs/getting_started/downloads.html \
`.bat` scripts have been included for simple compilation for Windows
#### Windows
In the root folder of the project (replacing <emsdk_env.bat path> with the path to your emsdk_env.bat file from your Emscripten setup)
```bash
./scripts/configure_backend.bat <emsdk_env.bat path>
./scripts/compile_backend.bat <emsdk_evn.bat path>
```
**Example:** `./scripts/configure_backend.bat C:\emsdk\emsdk_env.bat`
>If Emscripten was added to your system `PATH` you don't need to specify the path to `emsdk_env.bat`\
>**Example:** `./scripts/compile_backend.bat`

#### Linux/macOS
Currently no scripting files have been included\
However, compilation can be done via provided CMake files

### 3. Set up and run the frontend
In the root folder of the project:
```bash
cd frontend
npm install
npm run dev
```

## Generating Documentation
This project supports Doxygen documentation for C++ backend\
To generate documentation:
```bash
doxygen Doxyfile
```

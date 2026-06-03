[![Build](https://github.com/zGiuly/Colony-Finder/actions/workflows/build.yml/badge.svg)](https://github.com/zGiuly/Colony-Finder/actions/workflows/build.yml)

# Colony Finder

A tool for Elite Dangerous players to search and discover systems suitable for colonization.

## Features

- **Download System Data**: Automatically fetch the latest system database
- **Fast Search**: Search through millions of systems with indexed data
- **JSON Validation**: Stream-process and validate system data safely
- **Multi-platform**: Runs on Windows and Linux

## Technology

Built with C++ 20 and modern libraries:
- **UI**: ImGui with GLFW for graphics
- **Parsing**: simdjson for fast JSON processing
- **Download**: libcurl for HTTP requests
- **Compression**: zlib for gzip decompression

## Getting Started

### Requirements
- CMake 3.14+
- C++ 20 compiler
- OpenGL support

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be in `build/Release/ColonyFinder.exe` (Windows) or `build/ColonyFinder` (Linux).

## Usage

1. Launch the application
2. Download the system database
3. Wait for indexing to complete
4. Search for colonizable systems

## How It Works

Colony Finder uses a custom binary indexing system for fast searches. Instead of searching through raw JSON, it builds a compact CFIDX index file that leverages memory-mapped I/O and efficient bitfield encoding.

**Key features:**
- Memory-mapped access for O(1) random system lookups
- Bitfield filtering for star types and body types
- Streaming SAX parser for handling millions of systems
- Top-N heap-based result sorting

For technical details, see [ARCHITECTURE.md](ARCHITECTURE.md).

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE) for details.

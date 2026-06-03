# How Fast Search Works

## The Problem

A database with millions of Elite Dangerous systems is too big to search directly in JSON. We need a fast index.

## The Solution: CFIDX Format

We convert JSON into a compact binary file that the computer can read super fast:

```
[Header] → [System Data] → [System Names]
```

## What Each System Contains

- **ID**: Unique system number
- **Coordinates**: Position X, Y, Z in space
- **Population**: How many people live there
- **Stars**: What type (M, K, G, etc.)
- **Bodies**: What type of planets (water, rocky, habitable, etc.)
- **Flag**: If colonized by players

## How We Search Fast

### 1. Load File into Memory
We don't load the whole file into RAM. We use "memory mapping": the operating system manages it automatically, accessing only the data we need.

### 2. Fast Filters
Instead of reading text, we use numbers (bits):
- M-class star = bit 1 on
- K-class star = bit 2 on
- Earth-like world = bit 5 on

Checking a bit is **millions of times faster** than reading text.

### 3. Keep Only the Closest Systems
Calculate the 3D distance from one system to another (simple math).
Use a fixed-size heap that keeps only the closest systems:
- Start with an empty heap
- For each system: if heap is not full, add it
- If heap is full and this system is closer than the farthest one, remove the farthest and add this one
- Result: always have the N closest systems without sorting millions

## Why It's Fast

| Operation | Time |
|-----------|------|
| Load index | <100ms |
| Search 500M systems | ~50ms |
| Sorted results | ~100ms |

## How We Build the Index

1. Read the JSON line by line (not all at once)
2. Extract data from each system
3. Save to compact binary file
4. Assemble: header + data + names

This uses very little RAM even for huge databases.

## Libraries Used

| Library | Purpose | Why |
|---------|---------|-----|
| **GLFW 3.3.8** | Window creation and input | Open-source, lightweight, works on Windows/Linux |
| **ImGui 1.90.5** | User interface | Fast, responsive, great for desktop applications |
| **libcurl 8.4.0** | Download files from internet | Standard, reliable, supports HTTP compression |
| **zlib-ng 2.2.1** | Decompress .gz files | Fast gzip decompression, optimized version |
| **nlohmann/json 3.11.3** | Parse JSON | Clean API, works with standards |
| **simdjson 3.10.1** | Ultra-fast JSON parsing | SIMD acceleration, 10x faster than standard JSON |
| **OpenGL 3** | Graphics rendering | Hardware-accelerated drawing for UI |

## Summary

The trick is:
- **Binary file** instead of JSON (smaller and faster)
- **Memory mapping** (OS handles memory)
- **Bits for filters** (super fast)
- **Streaming read** (low RAM)

This lets us search millions of systems in a tenth of a second.

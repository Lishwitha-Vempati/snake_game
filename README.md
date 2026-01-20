# 🐍 Multithreaded Snake Game (C++)

A modern console-based **Snake Game** implemented in **C++17**, featuring:
- Real-time keyboard input using **multithreading**
- **Cross-platform** compatibility (Windows / macOS / Linux)
- Smooth **non-blocking controls**
- Clean and responsive ASCII interface

---

## 🧩 Features

✅ **Multithreaded Input Handling**
> The game runs input in a separate thread to ensure smooth and responsive controls.  
> Even at high snake speeds, input lag is eliminated.

✅ **Cross-Platform Support**
> Works on both **Windows** (via `<conio.h>`) and **Unix-based systems** (via `<termios.h>` and `<fcntl.h>`).

✅ **Dynamic Gameplay**
> - Random food generation  
> - Growing snake  
> - Wall and self-collision detection  
> - Scoring system

✅ **Thread-Safe Synchronization**
> Input direction is guarded by a mutex to avoid race conditions between rendering, logic, and input threads.



## 🎮 Controls

| Key | Action |
|-----|--------|
| `W` | Move Up |
| `A` | Move Left |
| `S` | Move Down |
| `D` | Move Right |
| `X` | Exit Game |

---

## ⚙️ Build Instructions

### 🧱 Prerequisites
- **C++17 or later**
- A terminal or console that supports ANSI clear commands (`clear` / `cls`)
- For Linux/macOS: `g++` or `clang++`
- For Windows: `MinGW` or MSVC

---

### 🐧 Build on Linux / macOS

```bash
# Clone the repository

# Compile
g++ -std=c++17 -pthread snake.cpp -o snake

# Run
./snake

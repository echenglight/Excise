# Neon Flux Garden

An interactive, shader-driven particle playground written in modern C++ with SFML. Move your mouse to sculpt neon energy streams, toggle attraction and repulsion, unleash energy bursts, and soak in a kaleidoscopic vortex rendered completely on the GPU.

## Features

- **Dynamic kaleidoscopic shader** backdrop that reacts to time, energy polarity, and mouse position.
- **GPU-accelerated particle ribbons** with additive blending and persistence trails.
- **Interactive controls** to toggle turbulence, color cycling, slow-motion, and attraction/repulsion modes.
- **Energy bursts** spawned from your cursor, letting you paint the scene in realtime.
- **Portable CMake project** targeting C++17 and SFML 2.5+.

## Controls

| Input            | Action                                   |
| ---------------- | ---------------------------------------- |
| Mouse move       | Sculpt the energy field                  |
| Left mouse       | Emit a burst of particles                |
| Right mouse      | Toggle attraction / repulsion            |
| `Space`          | Toggle turbulent vortex forces           |
| `C`              | Toggle chroma cycling                    |
| `S`              | Toggle slow-motion                       |
| `R`              | Reset the particle universe              |
| `Esc` / window X | Quit the experience                      |

## Build instructions

1. Install the SFML development libraries for your platform (2.5 or newer).
2. Configure the project with CMake, then build:

   ```bash
   cmake -B build
   cmake --build build
   ```

3. Run the executable (ensure the `resources/` directory sits alongside it):

   ```bash
   ./build/neon_flux_garden
   ```

On Windows, use the Visual Studio generator (or `cmake -G "Ninja Multi-Config"`) and launch `neon_flux_garden.exe` from the build output folder.

## Project layout

```
.
├── CMakeLists.txt         # Build configuration
├── resources
│   └── shaders
│       └── kaleido.frag   # GLSL fragment shader for the neon background
├── src
│   └── main.cpp           # Application entry point and particle playground
└── README.md              # You are here
```

## Notes

- The application uses fragment shaders; a GPU supporting GLSL 1.20+ (OpenGL 2.1) is required.
- The render texture is rebuilt on launch. If you resize the window in code, make sure to recreate the render texture accordingly.
- If SFML reports that shaders are unavailable, update your graphics drivers or try running on a discrete GPU.

Enjoy sculpting your own neon flux garden!

<!-- LOGO placeholder -->

# Multiversal Consciousness Game Engine

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue?style=for-the-badge&logo=cplusplus&logoColor=white)
![SDL3](https://img.shields.io/badge/SDL-3-green?style=for-the-badge&logo=sdl&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.20+-orange?style=for-the-badge&logo=cmake&logoColor=white)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

A modern, RAII-driven 2D puzzle-platformer engine where players navigate agents across **two parallel realities**, switching between them to solve physics-based puzzles. Built from the ground up with a custom Entity-Component System, it gives developers a clean, extensible foundation for building dual-world game mechanics without fighting the framework.

<!-- Screenshot: A side-by-side or overlay view of Reality A and Reality B showing different obstacle layouts, an agent being possessed, and the HUD displaying the active reality and abilities. -->
![Project Screenshot](path/to/screenshot.png)

---

## Table of Contents

- [About The Project](#about-the-project)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Configuration](#configuration)
- [Level File Format](#level-file-format)
- [Controls](#controls)
- [Roadmap](#roadmap)
- [License](#license)
- [Contact / Support](#contact--support)

---

## About The Project

### Motivation

Most 2D game engines treat the game world as a single, static space. Multiversal Consciousness explores a different design: **what if the same level existed in two realities at once**, with shared geometry, independent inventories, and reality-specific abilities? This engine provides the systems needed to build that kind of game — dual-reality state management, a possession mechanic that lets one player hop between agents, and an ability system tied to which reality you're in — so developers can focus on puzzle design instead of infrastructure.

### Key Features

- **Dual Reality System** — Two parallel game worlds (Reality A and Reality B) with synchronized shared elements (doors, switches, water levels) and reality-specific inventories and abilities.
- **Universal Possession** — A consciousness-switching mechanic that lets the player take control of any agent (1–9) at any time, with automatic camera tracking.
- **Quantum Loadout System** — Abilities (Axe, Keycard, DoubleJump, Dash, WaterWalk, PhaseShift) are assigned per-reality through quantum nodes, creating asymmetric puzzle-solving.
- **Interactive Obstacle System** — Polymorphic obstacle types (trees, locked doors, chasms, switches) that require specific abilities to overcome.
- **Physics Engine** — Gravity, AABB collision detection with slide resolution, ground detection, and special traversal mechanics (water walking, double jumping).
- **Tile-Based Level Format** — A human-readable `.level` file format using tile coordinates (1 tile = 32px), making level authoring straightforward without pixel math.

### Built With

![C++](https://img.shields.io/badge/C%2B%2B23-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![SDL3](https://img.shields.io/badge/SDL3-1D7C3F?style=flat-square)
![SDL3_ttf](https://img.shields.io/badge/SDL3__ttf-1D7C3F?style=flat-square)
![SDL3_image](https://img.shields.io/badge/SDL3__image-1D7C3F?style=flat-square)
![Catch2](https://img.shields.io/badge/Catch2_v3.4-red?style=flat-square)
![CMake](https://img.shields.io/badge/CMake_3.20+-064F8C?style=flat-square&logo=cmake&logoColor=white)

---

## Getting Started

### Prerequisites

| Requirement | Version |
|---|---|
| C++ Compiler | GCC 10+, Clang 12+, or MSVC 2019+ (C++23 support) |
| CMake | 3.20 or higher |
| SDL3 | Latest development libraries |
| SDL3_ttf | Latest development libraries |
| SDL3_image | Latest development libraries |

> Catch2 v3.4.0 is fetched automatically via CMake `FetchContent` — no manual installation required.

### Installation

#### Windows

```cmd
:: Set SDL3 installation path
set SDL3_DIR=C:\path\to\SDL3

:: Build the project
build.bat
```

#### Linux / macOS

```bash
# Set SDL3 installation path (if not in system paths)
export SDL3_DIR=/path/to/SDL3

# Build the project
chmod +x build.sh
./build.sh
```

#### Manual Build (All Platforms)

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/SDL3 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

---

## Usage

### Running the Game

```bash
# Linux / macOS
./build/MultiversalConsciousness

# Windows
build\Debug\MultiversalConsciousness.exe
```

The main executable loads levels sequentially (simple_example → verification_scenario → tutorial → movement → abilities_obstacles → beginner) with transitions between them.

### Running Tests

```bash
# Run the full test suite
./build/tests                    # Linux / macOS
build\Debug\tests.exe            # Windows

# Run via CTest (from the build/ directory)
ctest

# Run a single test case by name
./build/tests "test case name"

# Run tests matching a tag
./build/tests "[physics]"
```

### Programmatic Usage

The engine exposes a straightforward C++ API for creating games programmatically:

```cpp
#include "engine/GameEngine.h"
#include "engine/PossessionSystem.h"
#include "engine/PhysicsSystem.h"
#include "engine/Components.h"

// Initialize the engine
GameEngine engine;
engine.initialize_from_file("config.txt");

// Access ECS managers
auto& entities   = engine.get_entity_manager();
auto& components = engine.get_component_registry();
auto& systems    = engine.get_system_manager();

// Register systems
auto* possession = systems.register_system(std::make_unique<PossessionSystem>());
auto* physics    = systems.register_system(std::make_unique<PhysicsSystem>());

systems.initialize();

// Create an agent entity
EntityID agent = entities.create_entity();
components.add_component<Transform>(agent, {100.0f, 200.0f});
components.add_component<Agent>(agent, {1, false, 140.0f});
components.add_component<PhysicsComponent>(agent, {});
components.add_component<BoundingBoxComponent>(agent, {32.0f, 32.0f});

// Possess agent #1 and run
possession->possess_agent(1);
engine.run();
engine.shutdown();
```

---

## Configuration

Runtime settings are loaded from `config.txt` in the project root:

```ini
# Window settings
window_title="Multiversal Consciousness"
window_width=1280
window_height=720
fullscreen=false
vsync=true

# Rendering settings
tile_size=32
render_scale=1.0
```

---

## Level File Format

Levels are defined in `.level` files under the `levels/` directory using tile-based coordinates (1 tile = 32x32 pixels, origin at top-left):

```ini
[info]
name=My Level
description=A short description of the level

[agents]
# type, number, x, y, speed
TestAgent,1,5,10,140

[quantum_nodes]
# x, y, reality_a_item, reality_b_item, radius
8,9,key,double_jump,40

[environment]
# type, x, y, properties...
wall,4,11,width=13,height=1,solid=true,type=ground
door,8,17,locked=true,required_key=keycard
switch,14,16,target_type=door,target_id=all_doors
trigger,15,10,width=2,height=1,type=success_zone

[conditions]
# Win conditions
agent_position,1,x>500
agent_has_ability,1,double_jump
trigger_activated,success_zone,true
```

---

## Controls

| Key | Action |
|---|---|
| **A / D** | Move left / right |
| **W / Space** | Jump |
| **E** | Interact with nearby obstacle |
| **R** | Switch reality (A ↔ B) |
| **1–9** | Possess agent by number |
| **ESC** | Pause / Menu |

---

## Roadmap

- [x] Core ECS architecture (EntityManager, ComponentRegistry, SystemManager)
- [x] Dual reality system with synchronized shared elements
- [x] Physics engine with gravity, AABB collision, and slide resolution
- [x] Possession mechanic with camera tracking
- [x] Quantum loadout and ability system (6 ability types)
- [x] Interactive obstacle system (trees, doors, chasms, switches)
- [x] Tile-based level loader with 8 playable levels
- [x] Comprehensive test suite (20 test files via Catch2)
- [x] Plugin system with dynamic library loading
- [ ] PuzzleValidator integration with LevelLoader condition checking
- [ ] CI/CD pipeline (GitHub Actions for build + test)
- [ ] Audio system (sound effects and music)
- [ ] Level editor tooling

---

## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

---

## Contact / Support

- **Email:** [tran219jn@gmail.com](mailto:tran219jn@gmail.com)
- **Website:** [jasontran.pages.dev](https://jasontran.pages.dev/)

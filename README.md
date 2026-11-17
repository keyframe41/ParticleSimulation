
# Particle Simulation

![CI](https://github.com/keyframe41/ParticleSimulation/actions/workflows/ci.yml/badge.svg)

![simulation running](assets/demo.gif)

A real-time 2D physics engine in C++ and SFML that simulates tens of thousands of colliding particles, with moving and rotating obstacles. It was my first large project, and I scaled it from about 1,000 to nearly 90,000 particles at 60 FPS (480 Hz calculation, 60 Hz rendering) through a series of optimizations.

As my first foray into a large-ish project, I made a Verlet Integration-based physics solver to simulate particles moving around and interacting with each other and obstacles, with inspiration from 2 videos ([1](https://www.youtube.com/watch?v=lS_qeBy3aQI) and [2](https://www.youtube.com/watch?v=9IULfQH7E90)) on the same project by Pezzza's Work. I also made 2 videos on my process: [Part 1](https://www.youtube.com/watch?v=XL8B5nzNEOc) and [Part 2](https://www.youtube.com/watch?v=tqF883zXRA4&t=1213s), talking about the basic solver and collision handling, followed by some more advanced optimizations and new features. By skimming through these you can get a pretty good idea of how everything works.

Part 1 code is in `main/main_original.cpp`, `solvers/solver_original.hpp` and `renderers/renderer_original.hpp`.
Part 2 code uses `renderer_fast.hpp`, and `solver_final.hpp`. Some presets for the simulation can be found in the main folder.

## How it works

- **Velocity Verlet integration**, a second order integrator which is more stable (energy conservation speaking) than basic Euler integration. Each frame is split into 8 substeps (effectively 480 Hz), which keeps fast particles from going too far into obstacles or passing through without colliding.
- **Collisions** between particles, done by pushing them apart along the line between their centers, and also dependent on particles' mass (they are all set equal currently in the repo). Walls also bounce particles with a dampening factor.
- **Grid hashing** to speed up collisions by dividing the window into many small cells and updating the list of particles per cell every step, so each particle is only tested against particles in neighboring cells and runtime is no longer  `O(N^2)`.
- **Multithreading.** The grid is split into vertical slices (multiple grid cells wide, 1 slice per thread), and all collisions/forces are processed in two passes, first the even-numbered and then the odd-numbered. This way threads don't access neighboring cells at the same time and the simulation can be kept deterministic.
- **Circular and rectangular obstacles** which can move in two settings as well as rotate. To collide with a rotated box, a particle is transformed into that box's coordinate frame with a rotation matrix so it can be treated as an ordinary box with horizontal/vertical edges, and transformed back into the global frame with the inverse rotation (I like this, it was quite clever). The rectangular boxes can also have an optional durability setting so they fade out.
- **Rendering** using vertex arrays, so all particles are drawn using one call, same goes for obstacles. Trails can be added to particles.

## Performance

| Step | Particles at 60 FPS |
| --- | --- |
| Basic solver w/ basic grid hashing (Part 1) | ~1,000 |
| + Improved grid hashing | ~8,000|
| + Multithreading | ~12,000 |
| + Vertex-array rendering | ~88,000 |

(Measured on Macbook M1 Pro)

It's interesting that vertex arrays provide so much speedup, and the reason is that originally the particles were drawn with a very high number of vertices (34) compared to the new render which only deals with 4 vertices/particle and can also utilize the threading. Rendering probably took up a significant chunk of the time per frame originally, so by Amdahl's Law it didn't speed up that much, but once we made rendering faster the boost became more significant.

## Scenes

Each scene is a separate program in `main/`:

| Program | Scene |
| --- | --- |
| `main_new` | 88,888 rainbow particles filling the screen, with optional set of obstacles |
| `main_original` | Part 1 version, using the older solver and renderer |
| `galtonboard` | Particles fall through pegs to demonstrate a bell curve |
| `breakout1` | Atari Breakout-style scene |
| `breakout2` | Blocks of particles breaking and melting downward |
| `course` | A marble obstacle course, where red obstacles send particles back to the start |
| `trail` | Particles leaving trails |
| `media_generator` | Reads or writes particle positions to make videos and images, see below |

**Controls**: 
- left mouse button to pull particles toward the mouse
- right button to repel particles
- **Esc** to quit

Sometimes these controls (and extra ones) are commented out to improve performance.

## Files

| File | Contents |
| --- | --- |
| `particle.hpp` | Particle object with Velocity Verlet |
| `solvers/solver_final.hpp` | Final solver w/ grid hashing, threaded collisions, obstacles |
| `solvers/solver_original.hpp` | Part 1 solver |
| `renderers/renderer_fast.hpp` | Vertex-array renderer |
| `renderers/renderer_original.hpp` | Part 1 renderer |
| `obstacles/dot.hpp`, `obstacles/box.hpp` | Circular and rotating rectangular obstacles |
| `thread.hpp` | Thread pool |
| `utils/` | Math helpers and random number generation |
| `main/` | Final particle scenes |

## Building and running

### Requirements

- CMake 3.28 or newer
- A C++17 compiler
- CMake downloads and builds SFML automatically

On Linux, first install SFML's dependencies:

```bash
sudo apt-get install libxrandr-dev libxcursor-dev libudev-dev libfreetype-dev \
  libgl1-mesa-dev libegl1-mesa-dev
```

### Build

```bash
git clone https://github.com/keyframe41/ParticleSimulation.git
cd ParticleSimulation
cmake -S . -B build
cmake --build build --config Release
```

### Run

Each scene is built into `bin/`, for example:

```bash
./bin/main_new
./bin/galtonboard
```

### Images and videos from particles

`media_generator` can save every particle's final position, or load a precomputed color for each particle, and due to the simulation's deterministic design we can generate images or videos. This is how I made [Bad Apple](https://www.youtube.com/watch?v=th8wpz4RstM), as well as other images. It's slow and memory-heavy (took ~10 minutes to load iirc), and it needs a separate Python script (not included) that generates video frames and matches positions against them to produce `colors.txt`, so it isn't built by default:

```bash
cmake -S . -B build -DBUILD_MEDIA_GENERATOR=ON
cmake --build build --config Release
```

Without a `colors.txt`, the scenes fall back to rainbow colors.

## Credits

- **Pezzza's Work** who inspired the project (and also led to my channel taking off). The two-pass threaded collision scheme follows the approach shown in his video, and `thread.hpp` is adapted from his version [VerletSFML-Multithread](https://github.com/johnBuffer/VerletSFML-Multithread).
- **Font:** JetBrains Mono, under the SIL Open Font License ([assets/OFL.txt](assets/OFL.txt)).
- **Build setup:** based on SFML's [CMake project template](https://github.com/SFML/cmake-sfml-project).

## License

See [LICENSE.md](LICENSE.md).

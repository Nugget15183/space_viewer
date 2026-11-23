# Space Viewer

##IMPORTANT
If the map view of planets doesnt appear to be working, scroll on it, it updates the view and fixes it

A real-time solar system simulator written in C++20 and OpenGL 3.3. You observe the planets from Earth through a virtual telescope, with orbits, relative scales, rotation and lighting kept as physically faithful as possible. Only the telescope itself is allowed to be impossibly powerful.

<img width="1285" height="751" alt="Screenshot 2026-11-21 212605" src="https://github.com/user-attachments/assets/da938c21-ad77-4ab5-9d93-f09ab91ad5da" />
<img width="1279" height="743" alt="Screenshot 2026-11-22 210616" src="https://github.com/user-attachments/assets/30a33f69-24f5-4e55-baf1-0a83ac79e481" />
<img width="1919" height="1022" alt="Screenshot 2026-11-22 231056" src="https://github.com/user-attachments/assets/d7403dc3-a499-4ff8-8ec6-623b2fa33df0" />

## Features

**Astronomy**
- Planet positions from JPL's Keplerian elements, solved in double precision, with a simulation clock you can pause, speed up (up to years per second) or reset to now.
- True relative sizes and distances. Angular sizes are exact, so Jupiter is only about 45″ across and stays sub-pixel until you zoom in.
- Axial tilt and rotation from IAU pole and prime meridian data, including retrograde Venus and Uranus.
- Oblate planets (Jupiter and Saturn are visibly squashed).
- Saturn's rings, with the ring's shadow on the planet and the planet's shadow on the rings.
  **Rendering**
- Sun-lit shading using Lommel–Seeliger reflectance and per-planet limb darkening, so full-phase planets look like flat bright discs instead of shaded balls.
- Ray-marched atmospheres with forward scattering (Venus glows around its crescent near inferior conjunction).
- HDR render target, ACES tone mapping and sRGB-correct texturing.
  **Telescope optics**
- Atmospheric seeing, as blur plus shimmer, scaled by the angular size of a pixel. The air is calm at wide FOV and boils at high zoom, like a real telescope.
- Diffraction limit, atmospheric dispersion (red/blue fringing on the limb), a circular field stop and vignette.
- Focus pull: scrolling or slewing to a target defocuses the view, and the red, green and blue images drift back together as it comes into focus.
  **Interface (Dear ImGui)**
- Telescope panel: time controls, optics sliders, target buttons and a live table of distances and apparent sizes.
- Solar system map: top-down view from ecliptic north with the Sun at the center, orbit paths, linear or log scale, zoom, hover info (distance and light time) and click-to-point.
## Controls

| Input | Action |
|---|---|
| Left-drag | Look around (sensitivity scales with zoom) |
| Scroll | Zoom (FOV 0.005°–90°); each step briefly defocuses |
| `0`–`8` | Point at a body: 0 Sun, 1 Mercury, 2 Venus, 4 Mars, 5 Jupiter, 6 Saturn, 7 Uranus, 8 Neptune (3 is Earth, the observer) |
| `X` / `Z` | Speed up / slow down simulation time |
| `[` / `]` | Less / more atmospheric seeing |
| `V` | Toggle the circular field stop |
| `Tab` | Show or hide the telescope panel |
| `M` | Show or hide the solar system map |
| `F11` | Toggle fullscreen |

Scrolling or dragging over an ImGui window controls the window, not the sky.

## Building

### Requirements

- CMake 4.0 or newer
- A C++20 compiler (developed with MSVC 2022 and CLion on Windows)
- A GPU and driver that support OpenGL 3.3 core
  GLFW 3.5.1, GLAD (OpenGL 3.3 core), GLM and `stb_image.h` are vendored in the repo. Dear ImGui (v1.91.9) is fetched by CMake, so the first configure needs an internet connection.

### Build and run

```sh
cmake -S . -B build
cmake --build build --config Release
```

The program loads shaders and textures from a `resources/` folder **relative to its working directory**, so run it from a directory that contains `resources/`. The simplest way is to copy the folder next to the executable after every build, by adding this to `CMakeLists.txt`:

```cmake
add_custom_command(TARGET space_viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:space_viewer>/resources)
```

### Download
You can get the latest release [here](https://github.com/Nugget15183/space_viewer/releases/tag/alpha), just unzip the file and run the .exe

### Project layout

```
space_viewer/
├── CMakeLists.txt
├── packages/            vendored dependencies
│   ├── glad/
│   ├── glfw-3.5.1/
│   └── glm/
├── resources/           loaded at runtime (see below)
└── src/
    ├── main.cpp
    ├── engine.cpp/.h    window, render loop, post-processing, UI
    ├── orbits.h         orbital elements, Kepler solver, constants
    ├── util.h           logging, file/texture/shader helpers
    └── stb_image.h
```

### Resources

| File | Purpose |
|---|---|
| `main.vert`, `main.frag` | Planet, atmosphere and ring shading |
| `post.vert`, `post.frag`, `composite.frag` | Fullscreen pass, blur, and the final telescope/tonemap composite |
| `sun.jpg`, `mercury.jpg`, `venus.jpg`, `earth.jpg`, `mars.jpg`, `jupiter.jpg`, `saturn.jpg`, `uranus.jpg`, `neptune.jpg` | Equirectangular (2:1) color maps. They are named after the bodies and **are not included in this repo** |
| `saturn_ring.png` | One-pixel-tall radial strip with alpha; the left edge is the inner ring radius and the right edge is the outer radius |

Free 2k maps (for example from Solar System Scope) work well, but check each source's license before redistributing. For Venus, a cloud-top map looks much better than a radar surface map, since clouds are all you can see from Earth. A body without a texture renders black. The Earth texture is only used if you move the camera off Earth.

## How it works

**Precision.** Positions are simulated in `double` precision in kilometers in the ecliptic frame. Rendering is camera-relative: each body is drawn at a fixed distance from the camera with a scale chosen so its angular size is exact. Each body gets its own tight near/far planes and its own depth clear, drawn far to near, so depth precision is never a problem, even across 30 AU.

**Orbits.** `orbits.h` evaluates JPL's "Keplerian Elements for Approximate Positions of the Major Planets" (valid 1800–2050), solving Kepler's equation with Newton iteration. Those elements are accurate to arcseconds for the inner planets and up to several arcminutes for Jupiter and Saturn; see JPL's page for the exact error table. The "Earth" entry is the Earth–Moon barycenter.

**Orientation.** Each body's pole (RA/Dec) and prime meridian rate (`w0`, `wRate` in `engine.h`) come from the IAU working group on cartographic coordinates. The model matrix chains body-fixed → ICRF equatorial → ecliptic → OpenGL axes, in double precision.

**Atmospheres and rings.** Atmospheres are a slightly larger sphere drawn additively, ray-marched in the fragment shader with exponential density, a soft terminator and a Henyey–Greenstein phase function. Ring and planet shadows are computed analytically in the planet's local frame: a ray toward the Sun is intersected with the ring plane (sampling the ring texture's alpha) or with the oblate planet.

**Telescope.** Everything in the post-process is expressed in arcseconds, using `arcsec per pixel = FOV × 3600 / window height`. Blur width is the quadrature sum of seeing, the diffraction limit and the focus error. Shimmer, dispersion and the focus-pull color separation are pixel offsets derived from the same quantities.

## Configuration

| What | Where |
|---|---|
| Default seeing, exposure, vignette | `engine.h` (`seeingArcsec`, `exposure`, `vignette`) |
| Virtual aperture, shimmer strength | `engine::postProcess()` |
| Per-planet limb darkening and atmosphere | `setbody(...)` calls in `engine::init()` |
| Ring radii | `RING_INNER_KM`, `RING_OUTER_KM` in `orbits.h` |
| Texture longitude alignment | `w0` per body in `engine.h` |

## Troubleshooting

- **Black planets:** texture files are missing or misnamed, or the working directory has no `resources/` folder. The log prints a warning for each texture it can't find.
- **Shader errors like "version forced to 110":** the shader file wasn't found (empty source) or has a byte-order mark. The loader strips BOMs, but confirm the working directory first.
- **Everything is blurry:** at high zoom, seeing and diffraction are applied in real angular units. Lower the seeing with `[`, or look for the focus pull to settle after scrolling.
## Known limitations and ideas

- The observer sits at the Earth–Moon barycenter, with no geographic location, so there is no horizon, altitude-dependent seeing or refraction yet.
- No Moon or other moons, no stars, and no light-time, aberration or precession corrections.
- Possible next steps: a star field with scintillation, the Moon and the Galilean moons, observer location with a horizon, bloom, and a free-flying camera.
## Credits

- Orbital elements: JPL Solar System Dynamics, *Approximate Positions of the Planets* (E. M. Standish)
- Rotation and pole data: IAU Working Group on Cartographic Coordinates and Rotational Elements
- Libraries: [GLFW](https://www.glfw.org/), [GLAD](https://glad.dav1d.de/), [GLM](https://github.com/g-truc/glm), [Dear ImGui](https://github.com/ocornut/imgui), [stb_image](https://github.com/nothings/stb)
## License

Not yet specified.
 

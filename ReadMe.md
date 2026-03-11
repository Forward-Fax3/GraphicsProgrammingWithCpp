# GraphicsProgrammingWithCpp

This project is a continued evolution of my previous [OOPWithCpp](https://github.com/Forward-Fax3/OOP-with-Cpp) repository, now named `GraphicsProgrammingWithCpp` to better reflect its focus to now become a GPU Ray Tracer learning project for a different module of my second year university course.

C++ sandbox/learning project focused on OOP-style architecture with a Vulkan renderer and an SDL3 + Dear ImGui application layer. Then a CPU Ray Tracer implementation as a the main rendering backend which sends renderd Images to the Vulkan renderer for display.
The project uses Premake to generate Visual Studio solutions with multiple SIMD builds (SSE4.2, AVX2, AVX512). All SIMD builds must be built as the StartProj will link against them based on CPU detection at runtime.

## Features
- Vulkan rendering backend (uses `VULKAN_SDK`)
- SDL3 windowing/input (`OOPWithCpp/3rdParty/Projects/git/SDL`)
- Dear ImGui UI (`OOPWithCpp/3rdParty/Projects/git/ImGui`)
- `glm` math (`OOPWithCpp/3rdParty/HeaderOnly/git/GLM`)
- `stb_image` loading (`OOPWithCpp/3rdParty/HeaderOnly/git/stbImage`)
- Premake workspace with multiple SIMD builds (`SSE4.2`, `AVX2`, `AVX512`)
- Visual Studio generator support (`vs2022`, `vs2026`)
- CPU Ray Tracer rendering backend
- MSVC and Clang support (though I recommend Clang for better optimisations)

## CPU Ray Tracer Features
- spheres at the moment
- basic materials (diffuse, metal, dielectric)
- diffuse light sources
- single and multi-threaded rendering
- simple split BVH acceleration structure
- scenes
	- single red sphere
	- 2 grey spheres
	- Dielectric Test
	- Metal Test
	- earth
	- Ray Tracing in one weekend final render like scene

## Repository Layout
- `OOPWithCpp/` — main library/source
- `OOPWithCpp/3rdParty/` — third party dependencies (git submodules)
- `StartProj/` — console app that depends on the library builds
- `ShaderSrc/` — shader sources (Slang)
- `premake5.lua` — build generation script
- `Bin/` — build outputs (generated)

## Requirements
- Windows (Premake script links Windows system libs)
- Visual Studio 2022 or Visual Studio 2026
- Vulkan SDK installed and `VULKAN_SDK` environment variable set

## Getting Started (Visual Studio)
1. Clone the repo.
2. Install Vulkan SDK and verify `VULKAN_SDK` is set (e.g. `%VULKAN_SDK%/Include` exists).
3. Generate project files:
4. Open a command prompt in the repo root and run:
   ```sh
   Premake5/Premake5.exe vs2022
   ```
   or for Visual Studio 2026:
   ```sh
   Premake5/Premake5.exe vs2026
   ```
	or use the provided batch files:
   - `GenerateVS2022.bat`
   - `GenerateVS2026.bat`

## Problems
- You may need to run both `premake5 vs2022` and `premake5 vs2026` so that both the sln and slnx files are generated. For some reason, Premake does not want to generate the sln file when using the vs2026 action alone.
- If you resize the window before you have enabled the ray tracer, it will not update the ray tracer's resolution. Therefor making the image look distorted, this can be fixed by resizing the window again after enabling the ray tracer.
- some times when moving the camera the whole program will soft lock up, this is due to a deadlock in the ray tracer's multi-threading implementation.

## Notes
- This File has been created with the assistance of AI. Though I have reviewed and edited it.

# OOPWithCpp

C++ sandbox/learning project focused on OOP-style architecture with a Vulkan renderer and an SDL3 + Dear ImGui application layer. Then a CPU Ray Tracer implementation is added as a secondary rendering backend which sends renderd Images to the Vulkan renderer for display.
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
- basic materials (diffuse, dielectric)
- single and multi-threaded rendering
- scenes
	- single red sphere
	- 2 grey spheres
	- Dielectric Test

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
   premake5 vs2022
   ```
   or for Visual Studio 2026:
   ```sh
   premake5 vs2026
   ```
	or use the provided batch files:
   - `GenerateVS2022.bat`
   - `GenerateVS2026.bat`

## Problems
- You may need to run both `premake5 vs2022` and `premake5 vs2026` so that both the sln and slnx files are generated. For some reason, Premake does not want to generate the sln file when using the vs2026 action alone.
- If you resize the window before you have enabled the ray tracer, it will not update the ray tracer's resolution. Therefor making the image look distorted, this can be fixed by moving the camera or resizing the window again after enabling the ray tracer.

## Notes
- This File has been created with the assistance of AI. Though I have reviewed and edited it.

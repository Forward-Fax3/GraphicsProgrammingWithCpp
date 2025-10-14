project "SDL3_SSE2"
	location "ProjectFiles/SDL"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default"
	vectorextensions "SSE2"

	targetdir ("%{wks.location}/Bin/" .. output .. "/SDL")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/SDL")

	files
	{
		"SDL/src/*.cpp",
		"SDL/src/*.c",
		"SDL/src/*/windows/*.cpp",
		"SDL/src/*/windows/*.c",
		"SDL/src/*/*.cpp",
		"SDL/src/*/*.c",
		"SDL/src/audio/*/*.cpp",
		"SDL/src/audio/*/*.c",
		"SDL/src/*/hidapi/*.c",
		"SDL/src/main/generic/SDL_sysmain_callbacks.c",
		"SDL/src/camera/mediafoundation/SDL_camera_mediafoundation.c",
		"SDL/src/camera/dummy/SDL_camera_dummy.c",
		"SDL/src/gpu/vulkan/SDL_gpu_vulkan.c",
		"SDL/src/gpu/d3d12/SDL_gpu_d3d12.c",
		"SDL/src/video/yuv2rgb/*.c",
		"SDL/src/video/dummy/*.c",
		"SDL/src/video/offscreen/*.c",
		"SDL/src/io/generic/*.c",
		"SDL/src/storage/generic/*.c",
		"SDL/src/thread/generic/SDL_syscond.c",
		"SDL/src/thread/generic/SDL_sysrwlock.c",
		"SDL/src/joystick/virtual/*.c",
		"SDL/src/joystick/gdk/*.cpp",
		"SDL/src/render/*/*.c",
		"SDL/src/render/*/*.cpp",

		"SDL/include/*.hpp",
		"SDL/include/*.h",
		"SDL/src/*.hpp",
		"SDL/src/*.h",
		"SDL/src/*/windows/*.hpp",
		"SDL/src/*/windows/*.h",
		"SDL/src/*/*.hpp",
		"SDL/src/*/*.h",
		"SDL/src/audio/*/*.hpp",
		"SDL/src/audio/*/*.h",
		"SDL/src/*/hidapi/*.h",
		"SDL/src/gpu/vulkan/SDL_gpu_vulkan_vkfuncs.h",
		"SDL/src/video/yuv2rgb/*.h",
		"SDL/src/video/dummy/*.h",
		"SDL/src/video/offscreen/*.h",
		"SDL/src/io/generic/*.h",
		"SDL/src/storage/generic/*.h",
		"SDL/src/thread/generic/SDL_sysrwlock_c.h",
		"SDL/src/joystick/virtual/*.h",
		"SDL/src/render/*/*.h",
		"SDL/src/render/*/*.hpp",
	}

	removefiles
	{
		"SDL/src/*/ps2/*.cpp",
		"SDL/src/*/ps2/*.c",
		"SDL/src/*/psp/*.cpp",
		"SDL/src/*/psp/*.c",
		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.c",
		"SDL/src/render/ngage/SDL_render_ops.cpp",

		"SDL/src/*/ps2/*.hpp",
		"SDL/src/*/ps2/*.h",
		"SDL/src/*/psp/*.hpp",
		"SDL/src/*/psp/*.h",
		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.h",
	}

	includedirs
	{
        "SDL/src",
		"SDL/include",
		"SDL/include/build_config",
--		"SDL/include/**",
	}

	defines
	{
		"_WINDOWS",
		"SDL_DISABLE_SSE3",
		"SDL_DISABLE_SSE4_1",
		"SDL_DISABLE_SSE4_2",
		"SDL_DISABLE_AVX",
		"SDL_DISABLE_AVX2",
		"SDL_DISABLE_AVX512F"
	}

	filter { "system:windows" }
		systemversion "latest"
        
project "SDL3_AVX2"
	location "ProjectFiles/SDL"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default" 
	vectorextensions "AVX2"

	targetdir ("%{wks.location}/Bin/" .. output .. "/SDL")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/SDL")

	files
	{
		"SDL/src/*.cpp",
		"SDL/src/*.c",
		"SDL/src/*/windows/*.cpp",
		"SDL/src/*/windows/*.c",
		"SDL/src/*/*.cpp",
		"SDL/src/*/*.c",
		"SDL/src/audio/*/*.cpp",
		"SDL/src/audio/*/*.c",
		"SDL/src/*/hidapi/*.c",
		"SDL/src/main/generic/SDL_sysmain_callbacks.c",
		"SDL/src/camera/mediafoundation/SDL_camera_mediafoundation.c",
		"SDL/src/camera/dummy/SDL_camera_dummy.c",
		"SDL/src/gpu/vulkan/SDL_gpu_vulkan.c",
		"SDL/src/gpu/d3d12/SDL_gpu_d3d12.c",
		"SDL/src/video/yuv2rgb/*.c",
		"SDL/src/video/dummy/*.c",
		"SDL/src/video/offscreen/*.c",
		"SDL/src/io/generic/*.c",
		"SDL/src/storage/generic/*.c",
		"SDL/src/thread/generic/SDL_syscond.c",
		"SDL/src/thread/generic/SDL_sysrwlock.c",
		"SDL/src/joystick/virtual/*.c",
		"SDL/src/joystick/gdk/*.cpp",
		"SDL/src/render/*/*.c",
		"SDL/src/render/*/*.cpp",

		"SDL/include/*.hpp",
		"SDL/include/*.h",
		"SDL/src/*.hpp",
		"SDL/src/*.h",
		"SDL/src/*/windows/*.hpp",
		"SDL/src/*/windows/*.h",
		"SDL/src/*/*.hpp",
		"SDL/src/*/*.h",
		"SDL/src/audio/*/*.hpp",
		"SDL/src/audio/*/*.h",
		"SDL/src/*/hidapi/*.h",
		"SDL/src/gpu/vulkan/SDL_gpu_vulkan_vkfuncs.h",
		"SDL/src/video/yuv2rgb/*.h",
		"SDL/src/video/dummy/*.h",
		"SDL/src/video/offscreen/*.h",
		"SDL/src/io/generic/*.h",
		"SDL/src/storage/generic/*.h",
		"SDL/src/thread/generic/SDL_sysrwlock_c.h",
		"SDL/src/joystick/virtual/*.h",
		"SDL/src/render/*/*.h",
		"SDL/src/render/*/*.hpp",
	}

	removefiles
	{
		"SDL/src/*/ps2/*.cpp",
		"SDL/src/*/ps2/*.c",
		"SDL/src/*/psp/*.cpp",
		"SDL/src/*/psp/*.c",
		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.c",
		"SDL/src/render/ngage/SDL_render_ops.cpp",

		"SDL/src/*/ps2/*.hpp",
		"SDL/src/*/ps2/*.h",
		"SDL/src/*/psp/*.hpp",
		"SDL/src/*/psp/*.h",
		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.h",
	}

	includedirs
	{
        "SDL/src",
		"SDL/include",
		"SDL/include/build_config",
--		"SDL/include/**",
	}

	defines
	{
		"_WINDOWS",
		"__AVX__",
		"__AVX2__",
		"SDL_DISABLE_AVX512F"
	}

	filter { "system:windows" }
		systemversion "latest"

project "SDL3_AVX512"
	location "ProjectFiles/SDL"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default"
	vectorextensions "AVX512"

	targetdir ("%{wks.location}/Bin/" .. output .. "/SDL")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/SDL")

	files
	{
		"SDL/src/*.cpp",
		"SDL/src/*.c",
		"SDL/src/*/windows/*.cpp",
		"SDL/src/*/windows/*.c",
		"SDL/src/*/*.cpp",
		"SDL/src/*/*.c",
		"SDL/src/audio/*/*.cpp",
		"SDL/src/audio/*/*.c",
		"SDL/src/*/hidapi/*.c",
		"SDL/src/main/generic/SDL_sysmain_callbacks.c",
		"SDL/src/camera/mediafoundation/SDL_camera_mediafoundation.c",
		"SDL/src/camera/dummy/SDL_camera_dummy.c",
		"SDL/src/gpu/vulkan/SDL_gpu_vulkan.c",
		"SDL/src/gpu/d3d12/SDL_gpu_d3d12.c",
		"SDL/src/video/yuv2rgb/*.c",
		"SDL/src/video/dummy/*.c",
		"SDL/src/video/offscreen/*.c",
		"SDL/src/io/generic/*.c",
		"SDL/src/storage/generic/*.c",
		"SDL/src/thread/generic/SDL_syscond.c",
		"SDL/src/thread/generic/SDL_sysrwlock.c",
		"SDL/src/joystick/virtual/*.c",
		"SDL/src/joystick/gdk/*.cpp",
		"SDL/src/render/*/*.c",
		"SDL/src/render/*/*.cpp",

		"SDL/include/*.hpp",
		"SDL/include/*.h",
		"SDL/src/*.hpp",
		"SDL/src/*.h",
		"SDL/src/*/windows/*.hpp",
		"SDL/src/*/windows/*.h",
		"SDL/src/*/*.hpp",
		"SDL/src/*/*.h",
		"SDL/src/audio/*/*.hpp",
		"SDL/src/audio/*/*.h",
		"SDL/src/*/hidapi/*.h",
		"SDL/src/gpu/vulkan/SDL_gpu_vulkan_vkfuncs.h",
		"SDL/src/video/yuv2rgb/*.h",
		"SDL/src/video/dummy/*.h",
		"SDL/src/video/offscreen/*.h",
		"SDL/src/io/generic/*.h",
		"SDL/src/storage/generic/*.h",
		"SDL/src/thread/generic/SDL_sysrwlock_c.h",
		"SDL/src/joystick/virtual/*.h",
		"SDL/src/render/*/*.h",
		"SDL/src/render/*/*.hpp",
	}

	removefiles
	{
		"SDL/src/*/ps2/*.cpp",
		"SDL/src/*/ps2/*.c",
		"SDL/src/*/psp/*.cpp",
		"SDL/src/*/psp/*.c",
		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.c",
		"SDL/src/render/ngage/SDL_render_ops.cpp",

		"SDL/src/*/ps2/*.hpp",
		"SDL/src/*/ps2/*.h",
		"SDL/src/*/psp/*.hpp",
		"SDL/src/*/psp/*.h",
		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.h",
	}

	includedirs
	{
        "SDL/src",
		"SDL/include",
		"SDL/include/build_config",
--		"SDL/include/**",
	}

	defines
	{
		"_WINDOWS",
		"__AVX__",
		"__AVX2__",
		"__AVX512F__"
	}

	filter { "system:windows" }
		systemversion "latest"

project "ImGui_SSE2"
	location "ProjectFiles/ImGui"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default"
	vectorextensions "SSE2"

	targetdir ("%{wks.location}/Bin/" .. output .. "/ImGui")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/ImGui")

	files
	{
		"ImGui/*cpp",
		"ImGui/*.h",

		"ImGui/backends/imgui_impl_sdl3.cpp",
		"ImGui/backends/imgui_impl_sdl3.h",
		"ImGui/backends/imgui_impl_sdlgpu3.cpp",
		"ImGui/backends/imgui_impl_sdlgpu3.h"
	}

	includedirs
	{
        "ImGui",
		"SDL/include"
	}

	filter { "system:windows" }
		systemversion "latest"

project "ImGui_AVX2"
	location "ProjectFiles/ImGui"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default"
	vectorextensions "AVX2"

	targetdir ("%{wks.location}/Bin/" .. output .. "/ImGui")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/ImGui")

	files
	{
		"ImGui/*cpp",
		"ImGui/*.h",

		"ImGui/backends/imgui_impl_sdl3.cpp",
		"ImGui/backends/imgui_impl_sdl3.h",
		"ImGui/backends/imgui_impl_sdlgpu3.cpp",
		"ImGui/backends/imgui_impl_sdlgpu3.h"
	}

	includedirs
	{
        "ImGui",
		"SDL/include"
	}

	filter { "system:windows" }
		systemversion "latest"

project "ImGui_AVX512"
	location "ProjectFiles/ImGui"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default"
	vectorextensions "AVX512"

	targetdir ("%{wks.location}/Bin/" .. output .. "/ImGui")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/ImGui")

	files
	{
		"ImGui/*cpp",
		"ImGui/*.h",

		"ImGui/backends/imgui_impl_sdl3.cpp",
		"ImGui/backends/imgui_impl_sdl3.h",
		"ImGui/backends/imgui_impl_sdlgpu3.cpp",
		"ImGui/backends/imgui_impl_sdlgpu3.h"
	}

	includedirs
	{
        "ImGui",
		"SDL/include"
	}

	filter { "system:windows" }
		systemversion "latest"
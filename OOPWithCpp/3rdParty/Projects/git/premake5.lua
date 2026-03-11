project "SDL3_SSE4_2"
	location "ProjectFiles/SDL"
	kind "staticlib"
	staticruntime "off"
	floatingpoint "Default"
	vectorextensions "SSE2"

	targetdir ("%{wks.location}/Bin/" .. output .. "/SDL")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/SDL")

	files
	{
		"SDL/src/*.cpp",
		"SDL/src/*.c",
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
--		"SDL/src/joystick/virtual/*.c",
--		"SDL/src/joystick/gdk/*.cpp",
		"SDL/src/joystick/dummy/*.c",
		"SDL/src/render/*/*.c",
		"SDL/src/render/*/*.cpp",

		"SDL/include/*.hpp",
		"SDL/include/*.h",
		"SDL/src/*.hpp",
		"SDL/src/*.h",
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
		--		"SDL/src/joystick/virtual/*.h",
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
		"SDL_DISABLE_AVX512F",
		"HAVE_SYS_TYPES_H",
		"HAVE_STDIO_H",
		"HAVE_STDLIB_H",
		"HAVE_STDDEF_H",
		"HAVE_STDARG_H",
		"HAVE_STRING_H",
		"HAVE_MEMORY_H",
		"HAVE_STRINGS_H",
		"HAVE_WCHAR_H",
		"HAVE_INTTYPES_H",
		"HAVE_MATH_H",
		"HAVE_FLOAT_H",

		"SDL_JOYSTICK=OFF",
		"SDL_STATIC=1",
		"SDL_SHARED=0"
	}



	filter { "platforms:clang or system:linux" }
		buildoptions
		{
			"-msse4.2"
		}

	filter { "platforms:msc" }
		buildoptions
		{
			"/arch:SSE4.2"
		}

	filter { "system:windows" }
		systemversion "latest"
		files {
			"SDL/src/*/windows/*.cpp",
			"SDL/src/*/windows/*.c",
			"SDL/src/thread/windows/*.c",

			"SDL/src/*/windows/*.hpp",
			"SDL/src/*/windows/*.h",
			"SDL/src/thread/windows/*.h",
		}

		defines {
			"_WINDOWS",
		}

	filter { "system:linux" }
		files {
			"SDL/src/*/linux/*.cpp",
			"SDL/src/*/linux/*.c",
			"SDL/src/*/unix/*.cpp",
			"SDL/src/*/unix/*.c",
			"SDL/src/thread/pthread/*.c",
			"SDL/src/loadso/dlopen/*.c",
			"SDL/src/sensor/dummy/*.c",
			"SDL/src/video/wayland/*.c",
			"SDL/build/wayland-generated-protocols/*.c",

			"SDL/src/*/linux/*.hpp",
			"SDL/src/*/linux/*.h",
			"SDL/src/*/unix/*.hpp",
			"SDL/src/*/unix/*.h",
			"SDL/src/thread/pthread/*.h",
			"SDL/src/sensor/dummy/*.h",
			"SDL/src/video/wayland/*.h",
			"SDL/build/wayland-generated-protocols/*.h",
		}

		removefiles {
			"SDL/src/core/linux/SDL_fcitx.c",
			"SDL/src/core/linux/SDL_system_theme.c",
			"SDL/src/hidapi/linux/hid.c",
		}

		defines {
			"SDL_PLATFORM_LINUX",
			"SDL_THREAD_PTHREAD",
			"SDL_TIME_UNIX",
			"SDL_TIMER_UNIX",
			"SDL_LOADSO_DLOPEN",
			"SDL_VIDEO_VULKAN",
			"SDL_VIDEO_DRIVER_WAYLAND",
			"HAVE_SIGTIMEDWAIT"
		}

		includedirs {
			"SDL/build/wayland-generated-protocols",
		}

		links {
			"libwayland-client.so",
			"libwayland-cursor.so",
			"libwayland-egl.so",
			"libxkbcommon.so"
		}

	filter {}
        
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
--		"SDL/src/joystick/virtual/*.c",
--		"SDL/src/joystick/gdk/*.cpp",
		"SDL/src/joystick/dummy/*.c",
		"SDL/src/render/*/*.c",
		"SDL/src/render/*/*.cpp",

		"SDL/include/*.hpp",
		"SDL/include/*.h",
		"SDL/src/*.hpp",
		"SDL/src/*.h",
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
--		"SDL/src/joystick/virtual/*.h",
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
		"SDL_DISABLE_AVX512F",
		"HAVE_SYS_TYPES_H",
		"HAVE_STDIO_H",
		"HAVE_STDLIB_H",
		"HAVE_STDDEF_H",
		"HAVE_STDARG_H",
		"HAVE_STRING_H",
		"HAVE_MEMORY_H",
		"HAVE_STRINGS_H",
		"HAVE_WCHAR_H",
		"HAVE_INTTYPES_H",
		"HAVE_MATH_H",
		"HAVE_FLOAT_H",

		"SDL_JOYSTICK=OFF",
		"SDL_STATIC=1",
		"SDL_SHARED=0"
	}

	filter { "system:windows" }
		systemversion "latest"
		files {
			"SDL/src/*/windows/*.cpp",
			"SDL/src/*/windows/*.c",
			"SDL/src/thread/windows/*.c",

			"SDL/src/*/windows/*.hpp",
			"SDL/src/*/windows/*.h",
			"SDL/src/thread/windows/*.h",
		}

		defines {
			"_WINDOWS",
		}

	filter { "system:linux" }
		files {
			"SDL/src/*/linux/*.cpp",
			"SDL/src/*/linux/*.c",
			"SDL/src/*/unix/*.cpp",
			"SDL/src/*/unix/*.c",
			"SDL/src/thread/pthread/*.c",
			"SDL/src/loadso/dlopen/*.c",
			"SDL/src/sensor/dummy/*.c",
			"SDL/src/video/wayland/*.c",
			"SDL/build/wayland-generated-protocols/*.c",

			"SDL/src/*/linux/*.hpp",
			"SDL/src/*/linux/*.h",
			"SDL/src/*/unix/*.hpp",
			"SDL/src/*/unix/*.h",
			"SDL/src/thread/pthread/*.h",
			"SDL/src/sensor/dummy/*.h",
			"SDL/src/video/wayland/*.h",
			"SDL/build/wayland-generated-protocols/*.h",
		}

		removefiles {
			"SDL/src/core/linux/SDL_fcitx.c",
			"SDL/src/core/linux/SDL_system_theme.c",
			"SDL/src/hidapi/linux/hid.c",
		}

		defines {
			"SDL_PLATFORM_LINUX",
			"SDL_THREAD_PTHREAD",
			"SDL_TIME_UNIX",
			"SDL_TIMER_UNIX",
			"SDL_LOADSO_DLOPEN",
			"SDL_VIDEO_VULKAN",
			"SDL_VIDEO_DRIVER_WAYLAND",
			"HAVE_SIGTIMEDWAIT"
		}

		includedirs {
			"SDL/build/wayland-generated-protocols",
		}

		links {
			"libwayland-client.so",
			"libwayland-cursor.so",
			"libwayland-egl.so",
			"libxkbcommon.so"
		}

	filter {}

--project "SDL3_AVX512"
--	location "ProjectFiles/SDL"
--	kind "staticlib"
--	staticruntime "on"
--	floatingpoint "Default"
--	vectorextensions "AVX512"
--
--	targetdir ("%{wks.location}/Bin/" .. output .. "/SDL")
--	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/SDL")
--
--	files
--	{
--		"SDL/src/*.cpp",
--		"SDL/src/*.c",
--		"SDL/src/*/windows/*.cpp",
--		"SDL/src/*/windows/*.c",
--		"SDL/src/*/*.cpp",
--		"SDL/src/*/*.c",
--		"SDL/src/audio/*/*.cpp",
--		"SDL/src/audio/*/*.c",
--		"SDL/src/*/hidapi/*.c",
--		"SDL/src/main/generic/SDL_sysmain_callbacks.c",
--		"SDL/src/camera/mediafoundation/SDL_camera_mediafoundation.c",
--		"SDL/src/camera/dummy/SDL_camera_dummy.c",
--		"SDL/src/gpu/vulkan/SDL_gpu_vulkan.c",
--		"SDL/src/gpu/d3d12/SDL_gpu_d3d12.c",
--		"SDL/src/video/yuv2rgb/*.c",
--		"SDL/src/video/dummy/*.c",
--		"SDL/src/video/offscreen/*.c",
--		"SDL/src/io/generic/*.c",
--		"SDL/src/storage/generic/*.c",
--		"SDL/src/thread/generic/SDL_syscond.c",
--		"SDL/src/thread/generic/SDL_sysrwlock.c",
--		"SDL/src/joystick/virtual/*.c",
--		"SDL/src/joystick/gdk/*.cpp",
--		"SDL/src/render/*/*.c",
--		"SDL/src/render/*/*.cpp",
--
--		"SDL/include/*.hpp",
--		"SDL/include/*.h",
--		"SDL/src/*.hpp",
--		"SDL/src/*.h",
--		"SDL/src/*/windows/*.hpp",
--		"SDL/src/*/windows/*.h",
--		"SDL/src/*/*.hpp",
--		"SDL/src/*/*.h",
--		"SDL/src/audio/*/*.hpp",
--		"SDL/src/audio/*/*.h",
--		"SDL/src/*/hidapi/*.h",
--		"SDL/src/gpu/vulkan/SDL_gpu_vulkan_vkfuncs.h",
--		"SDL/src/video/yuv2rgb/*.h",
--		"SDL/src/video/dummy/*.h",
--		"SDL/src/video/offscreen/*.h",
--		"SDL/src/io/generic/*.h",
--		"SDL/src/storage/generic/*.h",
--		"SDL/src/thread/generic/SDL_sysrwlock_c.h",
--		"SDL/src/joystick/virtual/*.h",
--		"SDL/src/render/*/*.h",
--		"SDL/src/render/*/*.hpp",
--	}
--
--	removefiles
--	{
--		"SDL/src/*/ps2/*.cpp",
--		"SDL/src/*/ps2/*.c",
--		"SDL/src/*/psp/*.cpp",
--		"SDL/src/*/psp/*.c",
--		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.c",
--		"SDL/src/render/ngage/SDL_render_ops.cpp",
--
--		"SDL/src/*/ps2/*.hpp",
--		"SDL/src/*/ps2/*.h",
--		"SDL/src/*/psp/*.hpp",
--		"SDL/src/*/psp/*.h",
--		"SDL/src/hidapi/windows/hidapi_descriptor_reconstruct.h",
--	}
--
--	includedirs
--	{
--        "SDL/src",
--		"SDL/include",
--		"SDL/include/build_config",
----		"SDL/include/**",
--	}
--
--	defines
--	{
--		"_WINDOWS",
--		"__AVX__",
--		"__AVX2__",
--		"__AVX512F__"
--	}
--
--	filter { "platforms:clang" }
--		buildoptions
--		{
--			"-mavx512vbmi",
--			"-mavx512vbmi2"
--		}
--
--	filter { "system:windows" }
--		systemversion "latest"

project "ImGui_SSE4_2"
	location "ProjectFiles/ImGui"
	kind "staticlib"
	staticruntime "on"
	floatingpoint "Default"
	vectorextensions "SSE4.2"

	targetdir ("%{wks.location}/Bin/" .. output .. "/ImGui")
	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/ImGui")

	files
	{
		"ImGui/*cpp",
		"ImGui/*.h",

		"ImGui/backends/imgui_impl_sdl3.cpp",
		"ImGui/backends/imgui_impl_sdl3.h",
		"ImGui/backends/imgui_impl_vulkan.cpp",
		"ImGui/backends/imgui_impl_vulkan.h"
	}

	includedirs
	{
        "ImGui",
		"SDL/include",
	
		"%VULKAN_SDK%/Include"
	}

	filter { "platforms:clang or system:linux" }
		buildoptions
		{
			"-msse4.2"
		}
	filter { "platforms:msc" }
		buildoptions
		{
			"/arch:SSE4.2"
		}

	filter { "system:windows" }
		systemversion "latest"
	filter {}

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
		"ImGui/backends/imgui_impl_vulkan.cpp",
		"ImGui/backends/imgui_impl_vulkan.h"
	}

	includedirs
	{
        "ImGui",
		"SDL/include",
	
		"%VULKAN_SDK%/Include"
	}

	filter { "system:windows" }
		systemversion "latest"
	filter {}

--project "ImGui_AVX512"
--	location "ProjectFiles/ImGui"
--	kind "staticlib"
--	staticruntime "on"
--	floatingpoint "Default"
--	vectorextensions "AVX512"
--
--	targetdir ("%{wks.location}/Bin/" .. output .. "/ImGui")
--	objdir ("%{wks.location}/Bin/intermediate/" .. output .. "/ImGui")
--
--	files
--	{
--		"ImGui/*cpp",
--		"ImGui/*.h",
--
--		"ImGui/backends/imgui_impl_sdl3.cpp",
--		"ImGui/backends/imgui_impl_sdl3.h",
--		"ImGui/backends/imgui_impl_vulkan.cpp",
--		"ImGui/backends/imgui_impl_vulkan.h"
--	}
--
--	includedirs
--	{
--        "ImGui",
--		"SDL/include",
--
--		"%VULKAN_SDK%/Include"
--	}
--
--	filter { "platforms:clang" }
--		buildoptions
--		{
--			"-mavx512vbmi",
--			"-mavx512vbmi2"
--		}
--
--	filter { "system:windows" }
--		systemversion "latest"
newoption
{
	trigger = "cleanType",
	allowed =
	{
		"vs2022"
	},
	description = "select what type of clean you want to do."
}

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["SDL"] = "OOPWithCpp/3rdParty/Projects/git/SDL/include"
-- IncludeDir["glm"]  = "RTIAW/3rd Party/Git/glm/"
-- IncludeDir["CTPL"] = "RTIAW/3rd Party/Git/CTPL/"
-- IncludeDir["stb"]  = "RTIAW/3rd Party/Git/stb/"

workspace "OOPWithCpp"
	architecture "x86_64"

	configurations
	{
		"Debug",
		"Debug-Optimized",
		"Dist",
	}

	platforms
	{
		"msc",
		"clang",
--		"icc"
	}

	floatingpoint "Fast"
	floatingpointexceptions "off"

	if _ACTION == "clean" then
		os.rmdir("bin");
		filter { "options:cleanType=vs2022" }
			os.remove("OOPWithCpp.sln")
			os.remove("StartProj/StartProj.vcxproj")
			os.remove("StartProj/StartProj.vcxproj.user")
			os.remove("StartProj/StartProj.vcxproj.filters")

			os.remove("OOPWithCpp/OOPWithCpp*.vcxproj")
			os.remove("OOPWithCpp/OOPWithCpp*.vcxproj.user")
			os.remove("OOPWithCpp/OOPWithCpp*.vcxproj.filters")
			
			os.rmdir("OOPWithCpp/3rdParty/Projects/git/ProjectFiles")
			filter{}
	end

	output = "%{cfg.system}/%{cfg.buildcfg}/%{cfg.platform}"

	filter { "platforms:msc" }
		toolset "msc"
	filter { "platforms:clang" }
		toolset "clang"
	--filter { "platforms:icc" }
	--	toolset "icc"

	filter { "configurations:Debug" }
		runtime "Debug"
		defines
		{
			"DEBUG"
		}
		optimize "Off"
		symbols "On"
		inlining "Explicit"
		linktimeoptimization "Off"
	
	filter { "configurations:Debug-Optimized" }
		runtime "Release"
		defines 
		{
			"NDEBUG",
			"DEBUG_O",
			"EN_ENABLE_ASSERTS"
		}
		optimize "Speed"
		symbols "On"
		inlining "auto"
		linktimeoptimization "On"

	filter { "configurations:Dist" }
		runtime "Release"
		defines
		{
			"DIST",
			"NDEBUG"
		}
		linktimeoptimization "on"
		optimize "Speed"
		symbols "Off"
		inlining "auto"
		linktimeoptimization "On"
	filter {}

	intrinsics "On"
	startproject "StartProj"
	language "C++"
	cppdialect "C++latest"
	cdialect "c17"

	flags
	{
		"MultiProcessorCompile",
	}

	defines
	{
		"GLM_ENABLE_EXPERIMENTAL",
		"GLM_FORCE_DEFAULT_ALIGNED_GENTYPES",
		"GLM_FORCE_INLINE",
	}

	include "OOPWithCpp/3rdParty/Projects/git"

project "StartProj"
	location "%{prj.name}"
	kind "ConsoleApp"
	staticruntime "on"
	warnings "Extra"
	fatalwarnings "ALL"

	targetdir ("Bin/" .. output .. "/%{prj.name}")
	objdir ("Bin/intermediate/" .. output .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/src/**",
	}

	dependson
	{
		"OOPWithCppSSE4_2",
		"OOPWithCppAVX2",
		"OOPWithCppAVX512",
	}

	filter { "system:windows" }
		systemversion "latest"

project "OOPWithCppSSE4_2"
	location "OOPWithCpp"
	kind "sharedlib"
	staticruntime "on"
	warnings "Extra"
	fatalwarnings "ALL"
	vectorextensions "SSE4.2"

	targetdir ("Bin/" .. output .. "/OOPWithCpp")
	objdir ("Bin/intermediate/" .. output .. "/OOPWithCpp")

	files
	{
		"OOPWithCpp/src/**.cpp",

		"OOPWithCpp/src/**.hpp",
	}

	includedirs
	{
		"OOPWithCpp/src",
		"OOPWithCpp/src/**",
		
		"%{IncludeDir.SDL}",
--		"%{IncludeDir.glm}",
--		"%{IncludeDir.CTPL}",
--		"%{IncludeDir.stb}",
	}

	links
	{
		"SDL3_SSE4_2",
		"ImGui_SSE4_2",

		"Cfgmgr32.lib",
		"Winmm.lib",
		"SetupAPI.lib",
		"Imm32.lib",
		"Version.lib"
	}


	defines
	{
		"GLM_FORCE_SSE42",
		"SSE4_2",
	}

	filter { "platforms:clang" }
		buildoptions
		{
			"-msse4.2"
		}
	filter { "platforms:msc" }
		buildoptions
		{
			"/arch:SSE4.2"
		}
	
	
	filter { "platforms:clang AND configureation" }
		buildoptions
		{
			"-O3"
		}

	filter { "system:windows" }
		systemversion "latest"

project "OOPWithCppAVX2"
	location "OOPWithCpp"
	kind "sharedlib"
	staticruntime "on"
	warnings "Extra"
	fatalwarnings "ALL"
	vectorextensions "AVX2"

	targetdir ("Bin/" .. output .. "/OOPWithCpp")
	objdir ("Bin/intermediate/" .. output .. "/OOPWithCpp")

	files
	{
		"OOPWithCpp/src/**.cpp",

		"OOPWithCpp/src/**.hpp",
	}

	includedirs
	{
		"OOPWithCpp/src",
		"OOPWithCpp/src/**",
		
		"%{IncludeDir.SDL}",
--		"%{IncludeDir.glm}",
--		"%{IncludeDir.CTPL}",
--		"%{IncludeDir.stb}",
	}

	links
	{
		"SDL3_AVX2",
		"ImGui_AVX2",
		
		"Cfgmgr32.lib",
		"Winmm.lib",
		"SetupAPI.lib",
		"Imm32.lib",
		"Version.lib"
	}

	defines
	{
		"GLM_FORCE_AVX2",
		"GLM_FORCE_FMA",
		"AVX2"
	}

	filter { "system:windows" }
		systemversion "latest"

project "OOPWithCppAVX512"
	location "OOPWithCpp"
	kind "sharedlib"
	staticruntime "on"
	warnings "Extra"
	fatalwarnings "ALL"
	vectorextensions "AVX512"

	targetdir ("Bin/" .. output .. "/OOPWithCpp")
	objdir ("Bin/intermediate/" .. output .. "/OOPWithCpp")

	files
	{
		"OOPWithCpp/src/**.cpp",

		"OOPWithCpp/src/**.hpp",
	}

	includedirs
	{
		"OOPWithCpp/src",
		"OOPWithCpp/src/**",
		
		"%{IncludeDir.SDL}",
--		"%{IncludeDir.glm}",
--		"%{IncludeDir.CTPL}",
--		"%{IncludeDir.stb}",
	}

	links
	{
		"SDL3_AVX512",
		"ImGui_AVX512",
		
		"Cfgmgr32.lib",
		"Winmm.lib",
		"SetupAPI.lib",
		"Imm32.lib",
		"Version.lib"
	}

	defines
	{
		"GLM_FORCE_AVX2", -- GLM doesnt support AVX512 anymore so we use AVX2 instead though the compiler will still apply AVX512 optimizations
		"GLM_FORCE_FMA",
		"AVX2",
		"AVX512"
	}

	filter { "system:windows" }
		systemversion "latest"

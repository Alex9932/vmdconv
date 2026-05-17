workspace "vmdconv"
	configurations { "Debug", "Release" }
	architecture "x86_64"
	location "build"

	VCPKG_ROOT = os.getenv("VCPKG_ROOT") or "C:/vcpkg"
	VCPKG_TRIPLET = "x64-windows"

	includedirs {
		VCPKG_ROOT .. "/installed/" .. VCPKG_TRIPLET .. "/include"
	}
	libdirs {
		VCPKG_ROOT .. "/installed/" .. VCPKG_TRIPLET .. "/lib"
	}

project "vmdconv"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"

	dependson {  }
	links { "SDL3" }

	includedirs { "src" }

	files { "src/**.cpp", "src/**.c", "src/**.h" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "Off"

	links {  }
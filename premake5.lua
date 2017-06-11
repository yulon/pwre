workspace "pwre"
	language "C++"
	flags { "C++11" }
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}
	objdir("build/obj/%{cfg.system}")
	includedirs { "include" }

	project "pwre"
		kind "StaticLib"
		targetdir("lib/%{cfg.system}/%{cfg.platform}")
		files {
			"include/*.hpp", "include/*.h",
			"src/*.cpp", "src/*.hpp", "src/*.h",
			"src/win32/*.cpp", "src/win32/*.hpp",
			"src/x11/*.cpp", "src/x11/*.hpp"
		}

		configuration { "windows", "gmake" }
			targetprefix "lib"
			targetextension ".a"

		configuration "macosx"
			files { "src/cocoa/*.mm", "src/cocoa/*.hpp" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"
			targetsuffix ("_d")

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

	project "demo_blank"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}")
		files { "demo/blank.cpp" }
		libdirs { "lib/%{cfg.system}/%{cfg.platform}" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"
			targetsuffix ("_d")
			links { "pwre_d" }

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"
			links { "pwre" }

		configuration "windows"
			links { "user32" }

		configuration "macosx"
			linkoptions { "-framework Cocoa" }

		configuration "linux"
			links { "X11", "pthread" }

	project "demo_gl"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}")
		files { "demo/gl.cpp" }
		libdirs { "lib/%{cfg.system}/%{cfg.platform}" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"
			targetsuffix ("_d")
			links { "pwre_d" }

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"
			links { "pwre" }

		configuration "windows"
			links { "user32", "gdi32", "opengl32" }

		configuration "macosx"
			linkoptions { "-framework Cocoa", "-framework OpenGL" }

		configuration "linux"
			links { "X11", "pthread", "GL", "Xrender" }

	project "demo_gl_alpha"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}")
		files { "demo/gl_alpha.cpp" }
		libdirs { "lib/%{cfg.system}/%{cfg.platform}" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"
			targetsuffix ("_d")
			links { "pwre_d" }

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"
			links { "pwre" }

		configuration "windows"
			links { "user32", "gdi32", "opengl32" }

		configuration "macosx"
			linkoptions { "-framework Cocoa", "-framework OpenGL" }

		configuration "linux"
			links { "X11", "pthread", "GL", "Xrender" }

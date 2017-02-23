workspace "Pwre"
	language "C++"
	flags { "C++11" }
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}
	objdir("build/obj/%{cfg.system}")
	includedirs { "include" }

	project "pwre"
		kind "StaticLib"
		targetdir("lib/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "src/*.cpp", "src/*.hpp", "src/*.h", "include/*.hpp", "include/*.h" }
		includedirs { "deps" }

		configuration { "windows", "gmake" }
			targetprefix "lib"
			targetextension ".a"

		configuration "macosx"
			files { "src/*.mm" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

	project "demo_blank"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "demo/blank.cpp" }
		libdirs { "lib/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}" }
		links { "pwre" }

		configuration "windows"
			links { "user32" }

		configuration "macosx"
			linkoptions { "-framework Cocoa" }

		configuration "linux"
			links { "X11", "pthread" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

	project "demo_gl"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "demo/gl.cpp" }
		libdirs { "lib/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}" }
		links { "pwre" }

		configuration "windows"
			links { "user32", "gdi32", "opengl32" }

		configuration "macosx"
			linkoptions { "-framework Cocoa", "-framework OpenGL" }

		configuration "linux"
			links { "X11", "pthread", "GL", "Xrender" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

	project "demo_gl_alpha"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "demo/gl_alpha.cpp" }
		libdirs { "lib/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}" }
		links { "pwre" }

		configuration "windows"
			links { "user32", "gdi32", "opengl32" }

		configuration "macosx"
			linkoptions { "-framework Cocoa", "-framework OpenGL" }

		configuration "linux"
			links { "X11", "pthread", "GL", "Xrender" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

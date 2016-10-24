workspace "Pwre"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}
	objdir("build/obj/%{cfg.system}")
	includedirs { "include" }

	project "pwre"
		language "C"
		kind "StaticLib"
		targetdir("lib/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "src/*.c", "src/*.h", "include/*.h" }
		includedirs { "deps" }

		configuration { "windows", "gmake" }
			targetprefix "lib"
			targetextension ".a"

		configuration "macosx"
			files { "src/*.m" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

	project "demo_blank"
		language "C"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "demo/blank.c" }
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
		language "C"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "demo/gl.c" }
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
		language "C"
		kind "ConsoleApp"
		targetdir("bin/%{cfg.system}/%{cfg.platform}/%{cfg.buildcfg}")
		files { "demo/gl_alpha.c" }
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

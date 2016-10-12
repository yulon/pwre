newoption {
	trigger = "inst_prefix",
	value = "PATH",
	description = "[inst_prefix]/(bin or include or lib)"
}
local instPrefix = _OPTIONS["inst_prefix"] or ""

--[[
newoption {
	trigger = "inst_bin_suffix",
	value = "PATH",
	description = "/bin[inst_bin_suffix]"
}
local instBinSuffix = _OPTIONS["inst_bin_suffix"] or ""
]]

newoption {
	trigger = "inst_inc_suffix",
	value = "PATH",
	description = "/include[inst_inc_suffix]"
}
local instIncSuffix = _OPTIONS["inst_inc_suffix"] or ""

newoption {
	trigger = "inst_lib_suffix",
	value = "PATH",
	description = "/lib[inst_lib_suffix]"
}
local instLibSuffix = _OPTIONS["inst_lib_suffix"] or ""

newaction {
	trigger = "install",
	description = "Install files.",
	execute = function ()
		if instPrefix == "" then
			if os.is("windows") then
				MSYSTEM = os.getenv("MSYSTEM")
				if MSYSTEM == "MINGW32" then
					instPrefix = "/mingw32"
				elseif MSYSTEM == "MINGW64" then
					instPrefix = "/mingw64"
				end
			else
				instPrefix = "/usr/local"
			end
		end
		--cp_file("bin/*", "/bin" .. instBinSuffix)
		cp_file("include/*", "/include" .. instIncSuffix)
		cp_file("lib/*", "/lib" .. instLibSuffix)
	end
}

function cp_file (file, dest)
	local fs = os.matchfiles(file)
	if #fs > 0 then
		os.execute("sh -c \"cp -fvu " .. file .. " '" .. instPrefix .. dest .."/'\"")
	end
end

---------------------------------------------------------------------------

workspace "Pwre"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"native", "x64", "x32"}

	project "pwre"
		kind "StaticLib"
		language "C"
		includedirs { "src", "include" }
		files { "src/*.c", "src/*.h", "include/*.h" }
		targetdir("lib")

		configuration {"windows", "gmake" }
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
		kind "ConsoleApp"
		language "C"
		includedirs { "include" }
		libdirs { "lib" }
		files { "demo/blank.c" }
		targetdir("bin")
		links { "pwre" }

		configuration "windows"
			links { "user32" }

		configuration "macosx"
			kind "WindowedApp"
			linkoptions { "-framework Cocoa" }

		configuration "linux"
			links { "X11", "GL" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

	project "demo_gl"
		language "C"
		includedirs { "include" }
		libdirs { "lib" }
		files { "demo/gl.c" }
		targetdir("bin")
		links { "pwre" }

		configuration "windows"
			kind "WindowedApp"
			links { "user32", "gdi32", "opengl32" }

		configuration "macosx"
			kind "WindowedApp"
			linkoptions { "-framework Cocoa", "-framework OpenGL" }

		configuration "linux"
			kind "ConsoleApp"
			links { "X11", "GL" }

		configuration "Debug"
			defines { "DEBUG" }
			symbols "On"
			warnings "Extra"

		configuration "Release"
			defines { "NDEBUG" }
			optimize "Speed"

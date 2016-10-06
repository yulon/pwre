set_project("Pwre")

if is_mode("debug") then
	add_defines("DEBUG")
	set_symbols("debug")
	set_optimize("none")
end

if is_mode("release") then
	add_defines("NDEBUG")
	set_symbols("hidden")
	set_optimize("fastest")
	set_strip("all")
end

add_includedirs("include")

target("pwre")
	set_kind("static")

	set_targetdir("lib")

	add_includedirs("src")

	add_files("src/*.c")

	if is_plat("macosx") then
		add_files("src/*.m")
	end

	if is_plat("mingw") then
		after_build(function ()
			os.run("mv ./lib/pwre.lib ./lib/libpwre.a")
		end)
	end

target("demo_blank")

	add_deps("pwre")

	set_kind("binary")

	set_targetdir("bin")

	add_linkdirs("lib")

	add_files("demo/*.c")

	add_links("pwre")

	if is_plat("windows") then
		add_links("user32")
	end

	if is_plat("mingw") then
		add_ldflags("-static")
	end

	if is_plat("linux") then
		add_links("X11")
	end

	if is_plat("macosx") then
		add_ldflags("-framework Cocoa")
	end

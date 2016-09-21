set_project("UBWindow")

if is_mode("debug") then
	set_symbols("debug")
	set_optimize("none")
end

if is_mode("release") then
	set_symbols("hidden")
	set_optimize("fastest")
	set_strip("all")
end

add_includedirs("include")

target("demo_blank")

	set_kind("binary")

	set_targetdir("bin")

	add_linkdirs("lib")

	add_files("demo/*.c")

	add_links("ubwindow")

	if is_plat("linux") then
		add_links("X11")
	end

	if is_plat("macosx") then
		add_ldflags("-framework Cocoa")
	end

target("ubwindow")

	set_kind("static")

	set_targetdir("lib")

	add_includedirs("src")

	add_files("src/*.c")

	if is_plat("macosx") then
		add_files("src/*.m")
	end

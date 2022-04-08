project (
	'billigsweeper',
	'c',
	version: '0.1',
	default_options: [
		'warning_level=3',
		'c_std=gnu11',
	]
)

datadir = get_option ('prefix') / get_option ('datadir') / meson.project_name ()

# Create a 'config.h' file.
conf = configuration_data ()
conf.set_quoted ('MSW_VERSION', meson.project_version ())
conf.set_quoted ('MSW_GRAPHICS_PNG', datadir / 'graphics.png')
configure_file (
	input: 'config.h.in',
	output: 'config.h',
	configuration: conf,
)

# Search for necessary dependencies.
depends = [
	dependency ('sdl2'),
	dependency ('SDL2_image'),
]

# Define an executable.
executable (
	'billig-sweeper',
	'bsw.c',
	dependencies: depends,
	install: true,
)

# Install the graphics sprite.
install_data (
	'graphics.png',
	install_dir: datadir,
)

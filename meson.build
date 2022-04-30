project (
	'billigsweeper',
	'c',
	version: '1.0',
	default_options: [
		'warning_level=3',
		'c_std=gnu11',
	]
)

prefix = get_option ('prefix')
datadir = get_option ('datadir')
icondir = datadir / 'icons' / 'hicolor' / '256x256' / 'apps'

# Create a 'config.h' file.
conf = configuration_data ()
conf.set_quoted ('MSW_VERSION', meson.project_version ())
conf.set_quoted ('MSW_GRAPHICS_PNG', datadir / meson.project_name () / 'graphics.png')
conf.set_quoted ('MSW_ICON', icondir / 'billig-sweeper.png')
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

sources = [
	'src/main.c',
	'src/menu.c',
	'src/tile.c',
	'src/util.c',
	'src/video.c',
]

# Define an executable.
executable (
	'billig-sweeper',
	sources,
	include_directories: 'include',
	dependencies: depends,
	install: true,
)

# Install the graphics sprite.
install_data (
	'data/graphics.png',
	install_dir: prefix / datadir / meson.project_name (),
)

# Install icon.
install_data (
	'data/billig-sweeper.png',
	install_dir: icondir
)

# Install .desktop file.
install_data (
	'data/billig-sweeper.desktop',
	install_dir: prefix / datadir / 'applications'
)

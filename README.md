# Billig-Sweeper
This is a just another simple minesweeper clone.

# Usage
```
billig-sweeper [-h]
```

## Keybindings

| Key | Actions      |
|-----|--------------|
|  r  | Restart game |
|  q  | Quit         |

# Installation

## Dependencies
- meson (build-time)
- GIMP (only for editing `graphics.xcf`)
- libSDL2
- libSDL2_image

### Arch/Manjaro
```
sudo pacman -S base-devel meson sdl2 sdl2_image
```

### Debian/Ubuntu
```
sudo apt update
sudo apt install build-essential meson libsdl2-dev libsdl2-image-dev
```

### RHEL/Fedora
```
sudo dnf install gcc meson sdl2-devel sdl2_image-devel
```

## Building
```
git clone https://github.com/riscygeek/billig-sweeper
cd billig-sweeper
meson setup build
meson compile -C build
meson install -C build
```

# TODO
- Build an AppImage
- Add a menu

# Billig-Sweeper
This is a just another simple minesweeper clone.

![](data/Screenshots/bsw1.png)

# Usage
```
billig-sweeper [-h]
```

## Input

### Keyboard
| Action | Result           |
|--------|------------------|
| F1     | Open Help dialog |
| m      | Open menu        |
| r      | Restart game     |
| q      | Quit             |

### Mouse
| Action                | Result  |
|-----------------------|---------|
| Left-click            | Select  |
| Right-click           | Mark    |
| Middle-click + Motion | Panning |
| Space + Mouse Motion  | Panning |
| Scroll Wheel          | Zooming |

### Touchscreen
| Action     | Result  |
|------------|---------|
| Short Tap  | Select  |
| Long Tap   | Mark    |
| Swipe      | Panning |
| Pinching   | Zooming |


# Installation

## Flatpak
```
flatpak install xyz.stuerz.BilligSweeper
```

## Arch Linux (AUR)
AUR packages available: [billig-sweeper](https://aur.archlinux.org/packages/billig-sweeper), [billig-sweeper-git](https://aur.archlinux.org/packages/billig-sweeper-git)

## Building
### Dependencies
- git (build-time, for downloading tomlc99)
- meson (build-time)
- GIMP (only for editing `graphics.xcf`)
- libSDL2
- libSDL2_image

#### Arch/Manjaro
```
sudo pacman -S base-devel git meson sdl2 sdl2_image
```

#### Debian/Ubuntu
```
sudo apt update
sudo apt install build-essential git meson libsdl2-dev libsdl2-image-dev
```

#### RHEL/Fedora
```
sudo dnf install gcc git meson sdl2-devel sdl2_image-devel
```

### Build process
```
git clone https://github.com/riscygeek/billig-sweeper
cd billig-sweeper
git clone https://github.com/cktan/tomlc99 tomlc99
meson setup build
meson compile -C build
meson install -C build
```

Note:

This project must be installed to work,
if you just want to try it out,
you should add `--prefix=$PWD/tmp` to `meson setup build`.

# Contributing
It is encouraged to make issues and open pull requests.

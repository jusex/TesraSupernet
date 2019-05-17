
Debian
====================
This directory contains files used to package tesrad/tesra-qt
for Debian-based Linux systems. If you compile tesrad/tesra-qt yourself, there are some useful files here.

## tesra: URI support ##


tesra-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install tesra-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your tesraqt binary to `/usr/bin`
and the `../../share/pixmaps/tesra128.png` to `/usr/share/pixmaps`

tesra-qt.protocol (KDE)


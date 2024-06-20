## This is OctoXBPS, a powerful XBPS front end using Qt libs. 

![Main window](https://raw.githubusercontent.com/aarnt/octoxbps/master/octoxbps-mainwindow.png)

**OctoXBPS** is a Qt based GUI front end to the [XBPS](https://github.com/void-linux/xbps) package manager, derived from [OctoPkg](http://tintaescura.com/projects/octopkg).
It consists of a lxqt-sudo clone called [qt-sudo](https://github.com/aarnt/qt-sudo/) used to gain root privileges, a package browser application used 
to search, install, remove and update packages and a system tray icon [notifier](https://github.com/aarnt/octoxbps/tree/master/notifier) to notify the user about package changes.
The project is compatible with [Void Linux](https://voidlinux.org/).

### You can use XBPS to install the latest OctoXBPS version available in your distro:

```
# xbps-install -S octoxbps
```

### Follow the steps bellow to compile the latest source code (you'll need curl, git and qt5 packages):

```
$ git clone https://github.com/aarnt/qt-sudo
$ cd qt-sudo
$ qmake-qt5
$ make
# make install
$ cd ..
$ git clone https://github.com/aarnt/octoxbps
$ cd octoxbps/notifier
$ qmake-qt5
$ make
# make install
$ cd ..
$ qmake-qt5
$ make
# make install
```

### In order to run OctoXBPS main package browser:

```
$ octoxbps
```

### To execute OctoXBPS system tray notifier:

```
$ octoxbps-notifier
```

You'll also need "curl" and "sudo" packages installed and your user to be a member of the wheel group.


Enjoy!

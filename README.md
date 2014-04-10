huggle3-qt-lx
=============

Huggle 3 QT-LX is an anti-vandalism tool for use on Wikipedia and other Wikimedia projects, written in C++ (QT framework).  It is officialy supported for Windows (2000 or newer) and Linux (debian/ubuntu), but it may work on other distributions as well as MacOS.

[![Build Status](https://travis-ci.org/huggle/huggle3-qt-lx.png?branch=master)](https://travis-ci.org/huggle/huggle3-qt-lx)

Building
=========

Libraries you need to have to build:
* GCC (gcc g++ make)
* QT4 sdk (libqt4-dev libqt4-webkit libqt4-network qt4-qmake libqtwebkit-dev libqt4-dev-bin qt4-dev-tools)
* (optional) QT5 sdk (libqt5webkit5-dev qt5-default qtquick1-5-dev qtlocation5-dev qtsensors5-dev qtdeclarative5-dev)
* (optional) Python (libpython-dev)

Windows
-------------

* Download QT Creator from http://qt-project.org/downloads
* Download OpenSSL and Visual C++ 2008 Redistributables from http://slproweb.com/products/Win32OpenSSL.html
* Checkout this repository
* Enable powershell (run `Set-ExecutionPolicy unrestricted` as administrator in power shell)
* Execute huggle/configure.ps1 in powershell (from folder huggle)
* Open huggle.pro and hit build

Note in case you don't have power shell you need to do these 2 steps before you can build huggle:

* Create empty version.txt in folder huggle
* Copy definitions_prod.hpp to definitions.hpp

Linux
-------------

* checkout this repository
* cd REPO/huggle, execute

```sh
./configure
make
sudo make install
```

Fedora users may need to use ```./configure --qmake /usr/bin/qmake-qt4``` if it can't find qmake by itself.

MacOS
------------
* Install homebrew at http://brew.sh/
* Install Qt:
```sh
brew install qt
```
* Checkout this repository and execute
```sh
./configure
make
open huggle.app
```

Python
------------
To enable python engine you need to:

* open huggle.pro and uncomment / add python library
* open definitions.hpp and uncomment #define PYTHONENGINE

now rebuild huggle and pray

If you manage to compile it, you can insert your .py extensions to HUGGLEROOT/extensions, for more
information ask for help on our irc channel.

Documentation
=============

Developers: http://tools.wmflabs.org/huggle/docs/head

This is a documentation for users: https://meta.wikimedia.org/wiki/Huggle

Getting help
=============

We have a channel irc://chat.freenode.org/#huggle so if you need any kind of help please go there

Contributing
=============

Everyone is allowed to send the pull requests to this repository, and all regular contributors
get a developer access to directly push if they need it. If you want to join a huggle
developer team, please see https://meta.wikimedia.org/wiki/Huggle/Members and insert yourself

This repository is running CI using travis, if you want to change anything what doesn't require
sanity check, like documentation or comments, please append [ci skip] to your message

Reporting bugs
===============

Huggle is using google breakpad so when it crash a minidump should be generated.
In addition a core dump may be produced on linux systems. Both of these, or at
least minidump is very useful for developers to track the problems. So if the
minidump is generated during crash (you will see that in system log) you can
send this to developers in order to get the issue analysed and fixed.

Please use https://bugzilla.wikimedia.org/enter_bug.cgi?product=Huggle

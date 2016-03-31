huggle3-qt-lx
=============

Huggle 3 QT-LX is an anti-vandalism tool for use on Wikipedia and other MediaWiki sites, written in C++ (QT framework). It is officialy supported for Windows (2000 or newer), MacOS and Linux (debian/ubuntu).

[![Build Status](https://api.travis-ci.org/huggle/huggle3-qt-lx.png?branch=master)](https://travis-ci.org/huggle/huggle3-qt-lx/)

Building
=========

IMPORTANT: Since 3.1.19 you must download submodules in folder libs using git:

```
git submodule init
git submodule update
```

If you didn't download this repository using git, there should be a guide on where you can download the libraries from in the respective folders.

Libraries and tools you need to have to build:
* C++11 compiler
* [CMake](https://github.com/Kitware/CMake) 2.8.9 or higher is required
* QT4 sdk (libqt4-dev libqt4-webkit libqt4-network qt4-qmake libqtwebkit-dev libqt4-dev-bin qt4-dev-tools)
* (optional) QT5 sdk (libqt5webkit5-dev qt5-default qtquick1-5-dev qtlocation5-dev qtsensors5-dev qtscript5-dev qtdeclarative5-dev)
* (optional) Python (libpython-dev)

Windows
-------------
https://github.com/huggle/huggle3-qt-lx/wiki/Building-on-Windows

Linux
-------------
* checkout this repository
* cd REPO/huggle, execute

```sh
# IMPORTANT
# The --extension option works only if you cloned this repository using git
# if you downloaded a gzip file, you can only use it after downloading them
# by hand into 'huggle/extension_list' folder
./configure --extension --qt4 #you can use --qt5 in case you have it
cd huggle_release
make
sudo make install
```

MacOS
------------
https://github.com/huggle/huggle3-qt-lx/wiki/Building-on-MacOS

Python
------------
To enable python engine you need to:

On linux / mac:
run ./configure with --python option, for example
```
./configure --qt5 --python
```
Note: you need to have cmake 3.0.0 or newer for this to work

On windows you need to run cmake with -DHUGGLE_PYTHON=TRUE for it to work

Now rebuild Huggle and hope for the best!

If you manage to compile it, you can insert your .py extensions to HUGGLEROOT/extensions, for more
information ask for help on our irc channel.

Documentation
=============

Developers: https://tools.wmflabs.org/huggle/docs/head/

This is a documentation for users: https://meta.wikimedia.org/wiki/Huggle

Getting help
=============

We have an IRC-channel irc://chat.freenode.org/#huggle so if you need any kind of help please go there.

Contributing
=============

Everyone is allowed to send their pull requests to this repository, and all regular contributors
get a developer access to directly push if they need it. If you want to join the Huggle
developer team, please see https://meta.wikimedia.org/wiki/Huggle/Members and insert yourself.

This repository is running CI using travis, if you want to change anything which doesn't require
sanity check, like documentation or comments, please append [ci skip] to your message.

Reporting bugs
===============
Please use https://phabricator.wikimedia.org/maniphest/task/create/?projects=Huggle

Support us
===============

Your donations are welcome and help us focus on the development even more:

PayPal: http://tools.wmflabs.org/huggle/donate.htm
```
LTC:  LUQXStuTpmNnH9fCc9ABgZXjRs2WEPbSGn
Doge: DL54FhAuw6bJ2dEb9rBqE8G9Mz1X8Cmxr1
BTC:  19buq6BejVLqEKzDqBXYXEis2Ap1zaQd88
```

License
===============

Huggle 3 is licensed under GPL v3+, some contents of this repository may be licensed under
different license. See the local README or file headers for more information.

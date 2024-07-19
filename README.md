huggle3-qt-lx
=============

Huggle 3 is an anti-vandalism tool for use on [Wikipedia](https://wikipedia.org) and other [MediaWiki](https://mediawiki.org) sites, written in C++ (QT framework). It is officially supported for Windows (2000 or newer), MacOS and Linux (debian/ubuntu).

[![Build Status](https://api.travis-ci.org/huggle/huggle3-qt-lx.png?branch=master)](https://travis-ci.org/huggle/huggle3-qt-lx/)
[![Build status](https://ci.appveyor.com/api/projects/status/huy2phxnc95m77sl?svg=true)](https://ci.appveyor.com/project/benapetr/huggle3-qt-lx)

Building
=========

IMPORTANT: Since 3.1.19 you must download submodules in folder libs using git:

```sh
git submodule init
git submodule update
```

If you did not download this repository using git, there should be a guide on where you can download the libraries from in the respective folders.

Libraries and tools you need to have to build:

* C++11 compiler
* [CMake](https://github.com/Kitware/CMake) 2.8.9 or higher is required
* QT5 or QT6 sdk

[NEW] It is now possible to use a VirtualBox VM as a portable development environment for Huggle. [» Wiki page](https://github.com/huggle/huggle3-qt-lx/wiki/Portable-development-environment)

Windows
-------------

[» Wiki page](https://github.com/huggle/huggle3-qt-lx/wiki/Building-on-Windows)

Linux
-------------

* checkout this repository
* enter the repository folder, then execute:

```sh
# IMPORTANT
# The --extension option works only if you cloned this repository using git
# if you downloaded a gzip file, you can only use it after downloading them
# by hand into 'huggle/extension_list' folder
./configure --extension --web-engine
cd release
make
sudo make install
sudo ldconfig
```

MacOS
------------

[» Wiki page](https://github.com/huggle/huggle3-qt-lx/wiki/Building-on-MacOS)

WebKit vs Chromium
-------------------

Qt5.0 - Qt5.4 supports WebKit, which is a default backend for Huggle. Newer Qt only supports Chromium backend (code named WebEngine).

If you want to build Huggle with Qt newer than 5.4, you will have to enable it like this:

```sh
./configure --web-engine
```

Or alternatively if you are starting cmake by hand, just pass it the `-DWEB_ENGINE=true` parameter.

Documentation
=========

For developers: [source code documentation](https://tools.wmflabs.org/huggle/docs/head/) and [the repo's Wiki](https://github.com/huggle/huggle3-qt-lx/wiki) are available.

For users: [on metawiki](https://meta.wikimedia.org/wiki/Huggle) and [mediawiki](https://www.mediawiki.org/wiki/Manual:Huggle).

Getting help
=========

We have an IRC-channel: `irc://irc.libera.chat/#huggle`. If you need any kind of help, please go there.

Contributing
=========

Everyone is allowed to send their pull requests to this repository, and all regular contributors
get a developer access to directly push if they need it. If you want to join the Huggle
developer team, please see <https://meta.wikimedia.org/wiki/Huggle/Members> and insert yourself.

This repository is running CI using travis. If you want to change anything which doesn't require a
sanity check, like documentation or comments, please append [ci skip] to your message.

Reporting bugs
=========

Please use [Wikimedia's tracking platform, phabricator](https://phabricator.wikimedia.org/maniphest/task/create/?projects=Huggle).

Support us
=========

Your donations are welcome and help us focus on the development even more:

[PayPal](http://tools.wmflabs.org/huggle/donate.htm) (preferred), or :

```
LTC:  LfMDMWKqhiT45q4h1uduiDfjUZfiwGiCes
Doge: DMhK9EarJQaZrAHYinGReESL3CPvoMYcKK
BTC:  18YTu4mPqzaaRv5QarvMGRuPiH3ntk5ir2
```

License
=========

Huggle 3 is licensed under GPL v3+, some contents of this repository may be licensed under
different license. See the local README or file headers for more information.

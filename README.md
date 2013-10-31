huggle3-qt-lx
=============

Huggle 3 QT-LX is an anti-vandalism tool for use on Wikipedia and other Wikimedia projects, written in C++ (QT framework). It is supported for Windows and Linux


Building
=========

Libraries you need to have to build:
* QT sdk (libqt4-dev qt4-qmake libqtwebkit-dev libqt4-dev-bin qt4-dev-tools)
* (optional) Python (libpython-dev)

Windows:

* Download QT Creator from http://qt-project.org/downloads
* Checkout this repository
* Create empty version.txt in folder huggle
* Open huggle.pro in that and hit build

Linux:

* checkout this repository
* cd REPO/huggle, execute

```sh
./configure
make
sudo make install
```

Documentation
=============

Developers: http://tools.wmflabs.org/huggle/docs/head

This is a documentation for users: https://meta.wikimedia.org/wiki/Huggle

Getting help
=============

We have a channel #huggle @chat.freenode.org so if you need any kind of help please go there

Reporting bugs
===============

Please use https://bugzilla.wikimedia.org/enter_bug.cgi?product=Huggle


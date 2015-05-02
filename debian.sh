#!/bin/sh

cd huggle
git submodule init
git submodule update
debuild -us -uc

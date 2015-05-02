#!/bin/sh

cd huggle
git submodule init
git submodule update
if [ ! -f definitions.hpp ];then
    cp definitions_prod.hpp definitions.hpp
fi
debuild -us -uc

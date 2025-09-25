#! /bin/bash

./configure --qt6 --tests --extension --web-engine
cd release
make || exit 1

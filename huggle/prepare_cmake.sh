#!/bin/sh

if [ ! -f "definitions.hpp" ];then
  cp definitions_cmake.hpp definitions.hpp || exit 1
fi
exit 0

#!/bin/sh

if [ ! -f "definitions.hpp" ];then
  cp definitions_prod.hpp definitions.hpp || exit 1
fi
exit 0

#!/bin/sh

if [ ! -f "huggle_core/definitions.hpp" ];then
  cp huggle_core/definitions_prod.hpp huggle_core/definitions.hpp || exit 1
fi
exit 0

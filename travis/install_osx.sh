#! /bin/bash

if [ "$QTTYPE" = "4" ]; then
	sudo brew install qt4
fi

if [ "$QTTYPE" = "5" ]; then
	sudo brew install qt
fi


#! /bin/bash

#rvm install ruby-2.3.3
#rvm --default use 2.3.3

if [ "$QTTYPE" = "4" ]; then
	brew install qt4
fi

if [ "$QTTYPE" = "5" ]; then
	brew install qt
fi


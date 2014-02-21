#! /bin/bash

if [ "$QTTYPE" = "5" ]; then
	sudo add-apt-repository --yes ppa:ubuntu-sdk-team/ppa
fi

sudo apt-get update

#!/usr/bin/env zsh

cmake -S../ -B../../scenegraph-build -GXcode \
	-DCMAKE_APPLE_SILICON_PROCESSOR=arm64
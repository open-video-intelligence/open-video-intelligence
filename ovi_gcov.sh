#!/bin/sh

geninfo build/CMakeFiles/ovi.dir/src/ build/CMakeFiles/ovi.dir/src/*.gcno -o build/out.info --include "*open-video-*"
genhtml build/out.info -o build/html

geninfo build/plugins/ -o build/out2.info --include "*plugin*"
genhtml build/out2.info -o build/html2

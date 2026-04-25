#!/bin/sh
cd "$(dirname "$(readlink -f "$0")")"

rm -f out/fishyrecorder.zip
cd bindir
zip -r ../out/fishyrecorder.zip *

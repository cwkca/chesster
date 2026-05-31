#!/bin/sh

if !([[ -n "$1" ]] && [[ "$1" =~ [0-9] ]]); then
    echo "Specify size"
    exit 1
fi

if [ -z $(which rsvg-convert) ]; then
    echo "Install librsvg"
    exit 1
fi

cd pieces
DEST="png$1"
mkdir -p $DEST

for SVG in `ls svg/*`; do
  PNG=`echo $SVG | sed "s|svg/\(.*\)\.svg|$DEST/\1.png|"`
  rsvg-convert $SVG -w $1 -h $1 -o $PNG
done

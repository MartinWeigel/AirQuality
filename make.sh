#!/bin/sh

if [ -z "$1" ]
  then
    echo "USAGE: $0 <serialport>"
    exit
fi

PROJECT="../AirQuality"
BOARD="-b esp8266:esp8266:d1_mini:baud=921600"

echo "FILES TO HEADERS"
function file_to_c { # Args: file, var_name, output
    xxd -i $1 | sed 's/\([0-9a-f]\)$/\0, 0x00/' > $1'.h'
}

cd static/
    file_to_c "index.html"
    file_to_c "favicon.png"
    file_to_c "manifest.webmanifest"
cd ..

echo "COMPILING"
arduino-cli compile $BOARD --quiet $PROJECT

echo
echo "UPLOADING"
arduino-cli upload $BOARD -p $1 $PROJECT


screen $1 115200

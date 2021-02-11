#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $DIR

cd /

# Run tests
HAS_ERROR=false
  echo "Compiling HB-UNI-Sen-WEA"
  arduino-cli compile \
    --clean \
    --quiet \
    -b arduino:avr:pro:cpu=8MHzatmega328 \
    HB-UNI-Sen-WEA.ino
  [ $? -ne 0 ] && HAS_ERROR=true

# AES?
#    --build-property "build.extra_flags=-DUSE_AES -DHM_DEF_KEY=0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10 -DHM_DEF_KEY_INDEX=0" \

if $HAS_ERROR; then
  >&2 echo "Errors occurred!"
  exit 1
fi

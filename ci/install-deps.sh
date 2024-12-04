#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $DIR

# Libs included in Lib-Manager
LIBS=(
  "OneWire"
)

# Libs from Git-Repositories
GIT_LIBS=(
  "https://github.com/GreyGnome/EnableInterrupt"
  "https://github.com/rocketscream/Low-Power.git"
  "https://github.com/finitespace/BME280.git"
  "https://github.com/pa-pa/AskSinPP.git"
)

# Install Libs
arduino-cli lib install ${LIBS[*]}

for REPO_URL in ${GIT_LIBS[*]}; do
  REPO="$(basename $REPO_URL | cut -d. -f1)"
  echo "Install $REPO from Git"
  ( arduino-cli lib install --git-url ${REPO_URL} 1>/dev/null )&
done
wait

# Install Cores
arduino-cli core update-index
echo Install arduino:avr core
arduino-cli core install arduino:avr:1.8.2

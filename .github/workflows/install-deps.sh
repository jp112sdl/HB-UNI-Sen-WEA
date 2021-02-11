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
  "https://github.com/adafruit/Adafruit_Sensor"
  "https://github.com/adafruit/DHT-sensor-library"
  "https://github.com/spease/Sensirion.git"
  "https://github.com/adafruit/TSL2561-Arduino-Library"
  "https://github.com/adafruit/Adafruit-BMP085-Library"
  "https://github.com/finitespace/BME280.git"
  "https://github.com/claws/BH1750"
  "https://github.com/xoseperez/hlw8012.git"
  "https://github.com/bogde/HX711.git"
  "https://github.com/ZinggJM/GxEPD.git"
  "https://github.com/ZinggJM/GxEPD2.git"
  "https://github.com/adafruit/Adafruit-GFX-Library.git"
  "https://github.com/adafruit/Adafruit_BusIO.git"
  "https://github.com/adafruit/Adafruit_SHT31.git"
  "https://github.com/olikraus/U8g2_for_Adafruit_GFX.git"
  "https://github.com/FastLED/FastLED.git"
  "https://github.com/PaulStoffregen/SoftwareSerial.git"
  "https://github.com/DFRobot/DFRobotDFPlayerMini.git"
  "https://github.com/miguelbalboa/rfid.git"
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
arduino-cli core install arduino:avr
echo Install MightyCore:avr core
arduino-cli core install MightyCore:avr

# Symlink AskSinPP lib
echo Symlinking AskSinPP library
ln -sf $(realpath $DIR/..) $(realpath $DIR/Arduino/libraries/AskSinPP)

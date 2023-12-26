# Basic-DHT-sensor-with-Firebase
A basic IoT program that uses ESP8266 to read temperature and humidity from DHT sensor then sent to Firebase, a cloud-based database
# Installation
Download Arduino IDE then install the following libraries:
- LiquidCrystal I2C (by Frank de Brabander)
- Firebase ESP8266 Client (by Mobizt)
- DHT sensor library (by Adafruit)\
\
Install ESP8266 board on Arduino:
- https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/
- https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- https://sparks.gogo.co.nz/ch340.html
# Set up Firebase 
- Create a new project 
- Copy database credentials (API, URL, Database Secret, email, password)
- In the Firebase Console, navigate to the Database section, choose Realtime Database (start with test mode)
# Devices
- ESP8266
- Temperature and Humidity sensor DHT11/22
- LCD display screen I2C
- an LED
# Reference
https://github.com/mobizt/Firebase-ESP8266/tree/master/examples/DataRetaining

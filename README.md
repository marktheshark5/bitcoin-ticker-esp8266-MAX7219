# Bitcoin Ticker using esp8266 dev board and MAX7219 LED Matrix
 - NodeMCU (or other esp8266 based board)
 - MAX7219 (I used 32x8 version but code is adjustable for more displays)
 - some cobbled together code

# Example of the ticker working
https://user-images.githubusercontent.com/58120125/110657759-f4042080-8175-11eb-8ca8-06d362e79527.mov

# Features
- pulls data from CoinDesk free APIs 
- displays different messages based on percent changes on the day

# Customize to work for you
- change the GMT time offset to change when historical price data is used from CoinDesk
- change CoinDesk API to use your currency
- add in your WiFi SSID and password

# Reference guides I used along the way
Learning how to set up Wi-Fi and call APIs
- https://electrosome.com/calling-api-esp8266/


LED Matrix Guide
- https://www.makerguides.com/max7219-led-dot-matrix-display-arduino-tutorial/


NTPClient to get world time (my way of extracting the previous day
- https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/

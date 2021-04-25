# Aurora PVI Powercontrol
Feed-In Power Control using RS485 based Aurora protocol for PowerOne / ABB / Fimer Solar Inverters

This is supposed to run in the background on any Linux based computer (Tested on Raspberry Pi and Debian/Ubuntu).

Current value of electric power consumption (in watts / kilowatts) are recorded from any locally installed remote-read-out capable energy meter, the values are optionally filtered and then used to regulate the power output of any attached ABB / Fimer / Power One solar inverter using the Aurora protocol to exactly match the currently consumed power wattage.

This way, a zero-feed-in solar system can be realised.

The energy meter is attached using the ESP8266 based [Tasmota Smart Meter Interface](https://tasmota.github.io/docs/Smart-Meter-Interface) using any of the interfaces supported (D0 wired / infrared, Modbus etc.), the values are received via MQTT.

In case the MQTT or smart meter connection is lost, a watch-dog timer engages shuting down the solar inverter.

# Incomplete Testing - Do Not Use in Production

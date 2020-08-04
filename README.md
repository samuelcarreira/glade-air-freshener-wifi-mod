# Glade Air Freshener WiFi Mod
Simple Mod to add WiFi connectivity to a Glade Air Freshener Automatic Spray. Customize the spray interval and the enabled days/hours to prevent automatic triggers at night

## **WARNING: This is a work in progress!**

## Project motivation/description

It's all started after I became very annoyed each time I forgot to turn off the automatic air freshener before going to bed and then, I ear in my room the motor noise during the night. So my first objective was to add some kind of scheduled timer to prevent automatic triggering during night time hours. 
Also, I notice that the device needs two batteries with high capacity. If the battery level was a little low, the device cannot trigger the actuator and I will have to change the batteries with a medium wear level.
Another untoward limitation is the maximum time interval (you can only set between 9 and 36 minutes) and I wanted to set a custom interval so a refill spray can last more than 60 days.
After a bit of thinking, I decided to add a cheap microcontroller to control the automatic air freshener. I chose an ESP8266 because it will be easy and cheap to implement a minimal solution. With a WiFi-enabled microcontroller, I can automatically set the time from an NTP public server (no need to worry to set the time or add an RTC external module to the circuit), and I can change the settings easily on a web page from anywhere of my house (no need to add an LCD screen/push buttons and code a settings menu...). On the schematics page, you can see more details about my hardware choices.

___
*Glade® is a trademark of S.C. JOHNSON & SON, Inc.*

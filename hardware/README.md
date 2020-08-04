# Hardware Notes

### Power input
My choice was to power the device with a common USB wall charger. I don't need a 100% portable device and I have a power outlet near the air freshener. Also, I don't want to care about the battery charge or to put the ESP8266 in deep sleep mode.
The 5.1mm DC jack is a good choice because it's almost a standard and it's more robust than the micro-USB connector.

### MOSFET
Without any ultra-specific equipment, I've measured a peek of 800mA with my multimeter. So you need to use a proper MOSFET that can handle that peak current. On my electronics components stock, I've only had two types of DIP MOSFETs, so I had to choose the beefy 50N06 (50A, 60V N-CHANNEL) with the TO-220 package. You can choose a lower-spec MOSFET, but be careful about the peak current.
Also, I've added a pull-down resistor, and a small 220R resistor to protect the uC pin from MOSFET gate capacitance.

### DHT11
It's a very cheap and low precise sensor so I do not recommend it. I've only used it because I want to get rid of this old sensor (now I only use DHT12 or other more precise sensors). Because the whole purpose of this project is to be a low cost one, why not to add a cheap and "useless" sensor.

### DC-DC converter (5V to 3.3V)
My initial idea was to power the circuit with 3.3V but because I cannot directly power the Wemos ESP8266 module on the 3V3 pin (some modules can be powered from 3.3 pin) without changes to the board, I've decided to use 5V to power the microcontroller and add a cheap step down regulator to power the air freshener circuit with 3.3V (~2x 1.5V batteries). With a 5V circuit, I can power the DHT11 sensor and use a common USB charger with a USB-DC jack cable.

### ESP8266
My initial choice was to use the ESP-01 module. It's small and low cost. But even with an ESP-01 flash programmer board, it's annoying to program and only I have few GPIO pins to use. The cost of a Wemos D1 mini is only a few more bucks and I can use the ESP-01 module on a project where the available space for the electronics is a priority.


### RCWL-0516
With this cheap Microwave radar motion sensor, I can detect the human presence near the module and prevent an automatic trigger when someone is near. Manly, I want to prevent spray directly on anyone's face when he walks nearby or scare him with an unexpected spray trigger. I could use other types of sensors (like ultrasonic or IR), but I will need to change the Glade Plastic case (drill holes, etc.), and they are more expensive. The RCWL-0516 is cheap (0.56 euro), and I can fit it easily inside the case.


![Schematic](/hardware/Schematic_Glade&#32;Air&#32;Freshener&#32;WiFi&#32;Mod_Sheet_1_20200225012246.png)

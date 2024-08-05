# WSN
Humidity and Temperature Measuring/Monitoring for Smart Farming


How to use the code:

1. Download VSCode.

2. Download the PlatformIO Extension for VSCode.

3. Create a new project using PlatformIO for each program (Gateway, node1, node2) and copy the code into the main function.

3. Update the library and monitor speed for each code in the platformio.ini file of the newly created project:

- Gateway:

lib_deps = h2zero/NimBLE-Arduino@^1.4.1
thingsboard/ThingsBoard@^0.9.3
monitor_speed = 115200

- NODE1, NODE2:

lib_deps = h2zero/NimBLE-Arduino@^1.4.1
milesburton/DallasTemperature@^3.11.0
paulstoffregen/OneWire@^2.3.7
monitor_speed = 115200

4. Build the program and upload the code to ESP32

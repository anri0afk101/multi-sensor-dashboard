Arduino-based multi-sensor dashboard that measures temperature, ambient light, and distance, then shows the temperature on a 4‑digit 7‑segment display, logs data to serial (for PuTTY), and drives a warning LED + buzzer when an object is nearby.

Features
DHT11 temperature sensing (°C) with basic NaN fallback to 25 °C.
LDR-based light sensor with automatic “night light” LED if it’s dark.
HC-SR04–style ultrasonic distance measurement with object detection up to ~150 cm.
Variable-rate buzzer beeps depending on distance (continuous tone when very close).
4‑digit multiplexed 7‑segment display showing temperature with one decimal and a “C”.
1‑minute interval serial logging of time (in minutes), light resistance, and temperature for external logging (e.g., PuTTY).

Hardware Requirements
Arduino Uno.
DHT11 temperature sensor (3 module pins).
LDR (photoresistor) + fixed resistor (3 kΩ as in code).
Ultrasonic sensor (HC‑SR04).
Active buzzer.
4‑digit common-anode or common-cathode 7‑segment module (wired according to pin mapping in code and has 10 pins).
1 indicator LED for “dark” detection (light sensor).
Jumper wires and breadboard.

Pinout

DHT11
Data pin → A0
Power → 5 V
GND → GND

Make sure you use the Adafruit DHT library and declare DHT dht(DHTPIN, DHTTYPE); as in the sketch.

Light Sensor
LDR and 3 kΩ resistor form a voltage divider.
Middle node of the divider → A4 (PHOTON)
LED (for dark indication) → A3 (LED) via suitable resistor (e.g., 220 Ω).

Ultrasonic Sensor
trigPin → A1
echoPin → A2
Red LED (object detected) → digital pin 1 (redLED) via resistor (scrapped).
Buzzer → A5 (buz) via resistor / transistor driver as needed.

7-Segment Display
Segment pins (a–g + decimal point):
segA → 5
segB → 13
segC → 10
segD → 8
segE → 7
segF → 4
segG → 11
segPt (decimal point) → 9

Digit select pins:
Digit 1 (thousands) → 6
Digit 2 (hundreds) → 3
Digit 3 (tens) → 2
Digit 4 (ones) → 12

Adjust common-anode vs common-cathode wiring so that HIGH/LOW in the code matches your display. If your digits appear inverted or too dim, you may need to invert logic or add digit transistors.

Software Setup
Install Arduino IDE.
Install the “DHT sensor library” by Adafruit and its dependency “Adafruit Unified Sensor” via Library Manager.
Select your board and COM port.
Copy the provided sketch into a new Arduino sketch.
Upload to your board.
Open Serial Monitor (or connect via PuTTY at 9600 baud) to see log output.

How It Works (High Level)
Loop Overview
The main loop() does:
Reads the LDR voltage and computes resistance; if it exceeds DARK_MIN (10 kΩ), the “dark” LED turns on.
Reads DHT11 temperature every 500 ms; if reading fails (NaN), it falls back to 25 °C.
Triggers the ultrasonic sensor, measures pulse width, and converts to distance in centimeters.
Flags objectDetected if distance is between 0 and 150 cm, turns the red LED on/off accordingly.
Calls handleBuzzer() to update buzzer tone/pulse pattern based on current distance.
Calls displayTemperature(temperature) to multiplex and display the current temperature with one decimal place and a “C”.
Every 60 seconds, prints a log line of minutes since start, calculated light resistance, and temperature to serial.

Buzzer Logic
handleBuzzer() sets a pauseInterval according to distance:
≤ 5 cm: continuous ON.
≤ 10 cm: very fast beeping.
≤ 15 cm, 30 cm, 50 cm: progressively slower beeps.
No object or > 50 cm: buzzer off.
It uses previousMillis and buzzerState to implement non-blocking timing.

7-Segment Display
displayTemperature(float temp) breaks the temperature into tens, ones, and tenths using tens(), ones(), and tenths() helpers.
selectDigit() activates one digit at a time (multiplexing).
sendDigit() maps a number (0–9 or “C”) to segment patterns via functions zero(), one(), …, nine(), cee().
digitDelayUs controls how long each digit stays active; here it is 1000 μs per digit (≈4 ms full refresh).


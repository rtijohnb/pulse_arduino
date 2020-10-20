# pulse_arduino

Arduino code to read the pulse sensor and send data to Pi

## Features
1. After powering up, the onboard yellow LED will blink the code version number (1 second per blink)
2. The onboard yellow LED will blink each time a heartbeat is detected during live detection
3. The last 10 heartbeats are used to calculate the beats/minute, 10 heartbeats are detected to output a BPM reading
4. When a loss of a heartbeat is detected, the BPM will read 0 and not output a valid value until 10 new beats are detected
5. Heartbeats are detected based on the average reading of the last 3 seconds of samples, plus a threshold value
6. The reading, average reading, threshold, and bpm values are printed out at 115200bps on the USB serial interface at 150 times / second
7. If a heartbeat is not detected for 30 seconds, a pseudo heartbeat is output at a static 80bpm. The LED will blink 4 times / sec during pseudo output

## Local Demonstration
Using the Arduino IDE you can view the waveform via Tools->Serial Plotter  
You can also view the raw data under Tools->Serial Monitor  
Make sure the correct port is selected under Tools->Port  

## Steps To Flash Code On The XIAO Board
Note: further details with pictures: https://wiki.seeedstudio.com/Seeeduino-XIAO/

1. Download the arduino IDE
2. Click on File > Preference, and fill Additional Boards Manager URLs with the url: https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
3. Click Tools-> Board-> Boards Manager, search for "Seeeduino XIAO". Install the "Seeed SAMD Boards" manager.
4. After installing the board, click Tools->Board, find "Seeeduino XIAO M0" and select it. Now you have already set up the board of Seeeduino XIAO for Arduino IDE.
5. Select the correct port number under Tools->Port
6. Compile and upload the code by using Sketch->Upload

## Troubleshooting

If your board has a solid yellow light on and doesn't show up as a com port, you can reset the board by shorting the two pins near A0. For details see: https://wiki.seeedstudio.com/Seeeduino-XIAO/

## Connecting The Sensor
Purple Analog output - A0  
Red - 3v3  
Black - GND  

![xiao pinout](https://files.seeedstudio.com/wiki/Seeeduino-XIAO/img/Seeeduino-XIAO-pinout.jpg)

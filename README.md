# LoRaMESH

Radioenge Equipamentos de Telecomunicação

Library used to configure EndDevice LoRaMESH's GPIO pins.
Implemented functions are listed in LoRaMESH.h

Tested in Arduino Uno.

Files:

Example_masterslave_analog.ino
   - Initializes the software serial interface on the pins 6 (RX) and 7 (TX).
   - Reads the local device ID to check if it is a master or slave.
   - If it is a master:
      - Waits for messages from slaves and shows the payload on the monitor.
      - Replies each message with an acknowledge.
   - If it is a slave:
      - Configures the GPIO 5 as analog input.
      - Reads the analog inputs and sends to the master periodically.
      
Example_master_analog.ino
   - Initializes the software serial interface on the pins 6 (RX) and 7 (TX).
   - Configures the GPIOs 5 and 6 of a remote device as analog inputs.
   - Sends requests for analog values periodically every 2 s.
   - Shows the analog values on the monitor.

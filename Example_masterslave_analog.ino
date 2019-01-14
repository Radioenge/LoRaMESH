// LoRaMESH_Radioenge - Version: Latest 
#include <LoRaMESH.h>

/*
  Radioenge Equipamentos de Telecomunicações
  
  Application example using the EndDevice LoRaMESH Radioenge together with
  the Arduino Uno.
  
  This script:
    - Initializes the software serial interface on the pins 6 (RX) and 7 (TX).
    - Reads the local device ID to check if it is a master or slave.
    - If it is a master:
      - Waits for messages from slaves and shows the payload on the monitor.
      - Replies each message with an acknowledge.
    - If it is a slave:
      - Configures the GPIO 5 as analog input.
      - Reads the analog inputs and sends to the master periodically.
*/

/* Includes ---------------------- */
#include <SoftwareSerial.h>
#include <stdint.h>
#include <stdbool.h>

/* Defines ----------------------- */
#define CMD_ANALOG  50

/* Payload buffer */
uint8_t bufferPayload[MAX_PAYLOAD_SIZE] = {0};
uint8_t payloadSize = 0;

/* Local device ID */
uint16_t localId;

/* Remote device ID */
uint16_t remoteId;

/* Local device Unique ID */
uint32_t localUniqueId;

/* Local device Net */
uint16_t localNet;

/* Received command */
uint8_t command;

bool isMaster;

/* Value read on the analog input */
uint16_t AdcIn;

/* SoftwareSerial handles */
SoftwareSerial* hSerialCommands = NULL;

/* Initialization routine */
void setup() {
  delay(1000);
  Serial.begin(9600); /* Initialize monitor serial */
  
  /* Initialize SoftwareSerial */
  hSerialCommands = SerialCommandsInit(7, 6, 9600);
  hSerialCommands->listen();

  /* Gets the local device ID */
  if(LocalRead(&localId, &localNet, &localUniqueId) != MESH_OK)
    Serial.print("Couldn't read local ID\n\n");
  else
  {
    Serial.print("Local ID: ");
    Serial.println(localId);
    Serial.print("Local NET: ");
    Serial.println(localNet);
    Serial.print("Local Unique ID: ");
    Serial.println(localUniqueId, HEX);
    Serial.print("\n");
  }

  if(localId == 0) /* Is a master */
  {
    isMaster = true;
    Serial.print("\nMaster - will wait for push messages\n");
  }

  else /* Is a slave */
  {
    Serial.print("Slave - will read GPIO5 and send to master periodically\n");
    delay(500);
    /* Configures the device's analog pins */
    if(GpioConfig(localId, GPIO5, ANALOG_IN, PULL_OFF) != MESH_OK)
      Serial.print("Error configuring GPIO5\n\n");
  }
}

/* Main loop */
void loop() {

  if(isMaster == true) /* Operating as master */
  {
    if(ReceivePacketCommand(&remoteId, &command, bufferPayload, &payloadSize, 5000) == MESH_OK)
    {
      /* Treats command */
      // ...
      
      /* Sends to monitor */
      Serial.print("\nID: ");
      Serial.print(remoteId);
      Serial.print("\nGPIO5: ");
      Serial.print(float(bufferPayload[0] | (bufferPayload[1]<<8))*8.0586e-4);
      Serial.print("V\n");

      /* Replies the message - ACK*/
      bufferPayload[0] = 0;
      bufferPayload[1] = 0;

      PrepareFrameCommand(remoteId, CMD_ANALOG, bufferPayload, 2);  // Payload size = 2
      SendPacket();
    }
  }
  else  /* Operating as slave */
  {
    delay(2000);
    if(GpioRead(localId, GPIO5, &AdcIn) != MESH_OK)
      Serial.print("\nCommunication error\n\n");
    else
    {
      /* Sends to master */
      bufferPayload[0] = (uint8_t)AdcIn&0xFF;
      bufferPayload[1] = (uint8_t)(AdcIn>>8);

      PrepareFrameCommand(localId, CMD_ANALOG, bufferPayload, 2);  // Payload size = 2
      SendPacket();
      
      Serial.println(AdcIn);

      /* Optional - Wait for the ACK message from the master */
      // ReceivePacketCommand(&remoteId, &command, bufferPayload, &payloadSize, 5000)
    }
  }
}
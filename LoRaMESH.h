/*-------------------------------------------------------------------------------------

    MIT License

    Copyright (c) 2022 elcereza

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

-------------------------------------------------------------------------------------*/

#ifndef LoRaMESH_h
#define LoRaMESH_h

#include <Stream.h>

#define MAX_PAYLOAD_SIZE 232
#define MAX_BUFFER_SIZE 237

#define BW125                     0x00
#define BW250                     0x01
#define BW500                     0x02

#define SF_LoRa_7                 0x07
#define SF_LoRa_8                 0x08
#define SF_LoRa_9                 0x09
#define SF_LoRa_10                0x0A
#define SF_LoRa_11                0x0B
#define SF_LoRa_12                0x0C
#define SF_FSK                    0x00

#define CR4_5                     0x01
#define CR4_6                     0x02
#define CR4_7                     0x03
#define CR4_8                     0x04

#define LoRa_CLASS_A              0x00
#define LoRa_CLASS_C              0x02

#define LoRa_WINDOW_5s            0x00
#define LoRa_WINDOW_10s           0x01
#define LoRa_WINDOW_15s           0x02

#define LoRa_GPIO_MODE_READ       0x00
#define LoRa_GPIO_MODE_CONFIG     0x02
#define LoRa_GPIO_MODE_WRITE      0x01

#define LoRa_GPIO0                0x00
#define LoRa_GPIO1                0x01 
#define LoRa_GPIO2                0x02 
#define LoRa_GPIO3                0x03
#define LoRa_GPIO4                0x04
#define LoRa_GPIO5                0x05
#define LoRa_GPIO6                0x06
#define LoRa_GPIO7                0x07 

#define LoRa_NOT_PULL             0x00
#define LoRa_PULLUP               0x01
#define LoRa_PULLDOWN             0x02

#define LoRa_INOUT_DIGITAL_INPUT  0x00
#define LoRa_INOUT_DIGITAL_OUTPUT 0x01
#define LoRa_INOUT_ANALOG_INPUT   0x03

#define LoRa_LOGICAL_LEVEL_LOW    0x00
#define LoRa_LOGICAL_LEVEL_HIGH   0x01

class LoRaMESH{
    public:
        bool debug_serial = false;
        typedef struct
        {
            uint8_t buffer[MAX_BUFFER_SIZE];
            uint8_t size;
            bool command;
        } Frame_Typedef;

        uint8_t bufferPayload[MAX_PAYLOAD_SIZE] = {0};
        uint8_t payloadSize = 0;

        uint16_t localId = 0;
        uint32_t localUniqueId;
        uint8_t command;
        bool isMaster;

        Frame_Typedef frame;
        uint16_t deviceId = -1;
        uint16_t deviceNet = -1;
        uint32_t deviceUniqueId = -1;

        uint32_t registered_password;

        uint8_t BW, SF, CR, LoRa_class, LoRa_window;

        typedef enum
        {
            MESH_OK,
            MESH_ERROR
        } MeshStatus_Typedef;
        Stream*  SerialLoRa;

        LoRaMESH(Stream *_SerialLoRa){
            SerialLoRa = _SerialLoRa;
        }

        uint16_t ComputeCRC(uint8_t* data_in, uint16_t length){
            uint16_t i;
            uint8_t bitbang, j;
            uint16_t crc_calc;

            crc_calc = 0xC181;
            for(i=0; i<length; i++)
            {
                crc_calc ^= (((uint16_t)data_in[i]) & 0x00FF);
                
                for(j=0; j<8; j++)
                {
                bitbang = crc_calc;
                crc_calc >>= 1;
                
                if(bitbang & 1)
                {
                    crc_calc ^= 0xA001;
                }
                }
            }
            return (crc_calc&0xFFFF);
        }

        MeshStatus_Typedef PrepareFrameCommand(uint16_t id, uint8_t command, uint8_t* payload, uint8_t payloadSize){
            if((id < 0)) return MESH_ERROR;
            if(command < 0) return MESH_ERROR;
            if(payload < 0) return MESH_ERROR;
            if(payloadSize < 0) return MESH_ERROR;
            

            uint16_t crc = 0;

            frame.size = payloadSize + 5;
            
            frame.buffer[0] = id&0xFF;
            frame.buffer[1] = (id>>8)&0xFF;
            
            frame.buffer[2] = command;
            
            if((payloadSize >= 0) && (payloadSize < MAX_PAYLOAD_SIZE))
            {
                memcpy(&(frame.buffer[3]), payload, payloadSize);
            
                crc = ComputeCRC((&frame.buffer[0]), payloadSize+3);
                frame.buffer[payloadSize+3] = crc&0xFF;
                frame.buffer[payloadSize+4] = (crc>>8)&0xFF;
            }
            else
            {
                memset(&frame.buffer[0], 0, MAX_BUFFER_SIZE);
                return MESH_ERROR;
            }

            frame.command = true;

            return MESH_OK;
        }

        MeshStatus_Typedef PrepareFrameTransp(uint16_t id, uint8_t* payload, uint8_t payloadSize)
        {
            uint8_t i = 0;

            if(payload < 0x00) return MESH_ERROR;
            if(id > 1023) return MESH_ERROR;
            if(deviceId < 0x00) return MESH_ERROR;
            
            if((id != 0) && (deviceId == 0)) 
            {
                frame.size = payloadSize + 2;
                frame.buffer[i++] = id&0xFF;
                frame.buffer[i++] = (id>>8)&0x03;
            }
            else
            {
                frame.size = payloadSize;
            }
            
            if((payloadSize >= 0) && (payloadSize < MAX_PAYLOAD_SIZE))
            {
                memcpy(&frame.buffer[i], payload, payloadSize);
            }
            else
            {
                memset(&frame.buffer[0], 0, MAX_BUFFER_SIZE);
                return MESH_ERROR;
            }

            frame.command = false;

            return MESH_OK;
        }

        MeshStatus_Typedef SendPacket(){
            if(frame.size == 0) return MESH_ERROR;
            if(debug_serial)
            {
                Serial.print("TX: ");
                printHex(frame.buffer, frame.size);
                
            }

            SerialLoRa->write(frame.buffer, frame.size);

            return MESH_OK;
        }




        MeshStatus_Typedef ReceivePacketCommand(uint16_t* id, uint8_t* command, uint8_t* payload, uint8_t* payloadSize, uint32_t timeout){
            uint16_t waitNextByte = 500;
            uint8_t i = 0;
            uint16_t crc = 0;

            if(id < 0x00) return MESH_ERROR;
            if(command < 0x00) return MESH_ERROR;
            if(payload < 0x00) return MESH_ERROR;
            if(payloadSize < 0x00) return MESH_ERROR;

            while( ((timeout > 0 ) || (i > 0)) && (waitNextByte > 0) )
            {
                if(SerialLoRa->available() > 0)
                {
                    byte a = SerialLoRa->read();
                    frame.buffer[i++] = a;
                    waitNextByte = 200;
                }
                
                if(i > 0){
                    waitNextByte--;
                }
                timeout--;
                delay(1);
            }
            
            if(i > 0){
                Serial.print("RX: ");
                printHex(frame.buffer, i);
            }

            if((timeout == 0) && (i == 0)) return MESH_ERROR;
            crc = (uint16_t)frame.buffer[i-2] | ((uint16_t)frame.buffer[i-1] << 8);
            if(ComputeCRC(&frame.buffer[0], i-2) != crc) return MESH_ERROR;
            *id = (uint16_t)frame.buffer[0] | ((uint16_t)frame.buffer[1] << 8);
            *command = frame.buffer[2];
            *payloadSize = i-5;
            memcpy(payload, &frame.buffer[3], i-5);
            
            return MESH_OK;
        }

        bool localread(){
            uint8_t b = 0;
            
            bufferPayload[b] = 0x00;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x00;

            PrepareFrameCommand(0, 0xE2, bufferPayload, b + 1);
            SendPacket();
            
            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK){
                if(command == 0xE2)
                {
                    registered_password = bufferPayload[1] << 8;
                    registered_password += bufferPayload[0];
                    
                    localUniqueId = bufferPayload[2];
                    localUniqueId = bufferPayload[3] + (localUniqueId << 8);
                    localUniqueId = bufferPayload[4] + (localUniqueId << 8);
                    localUniqueId = bufferPayload[5] + (localUniqueId << 8);
                    
                    return true;
                }
            } 
            return false;
        }

        void begin(bool _debug_serial = false){
            debug_serial = _debug_serial;
            localread();
            localId = (uint16_t)frame.buffer[0] | ((uint16_t)frame.buffer[1] << 8);
        }

        bool setnetworkId(uint16_t id){
            uint8_t b = 0;
            
            bufferPayload[b] = 0x00;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = (uint8_t)(localUniqueId >> 24);
            bufferPayload[++b] = (uint8_t)(localUniqueId >> 16);
            bufferPayload[++b] = (uint8_t)(localUniqueId >> 8);
            bufferPayload[++b] = (uint8_t)(localUniqueId & 0xFF);
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x04;

            PrepareFrameCommand(id, 0xCA, bufferPayload, b + 1);
            SendPacket();
            
            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK)
            {
                if(command == 0xCA)
                return true;
            } 
            return false;
        }

        bool setpassword(uint32_t password){
            if(password < 0x00 || password < 0x00 || password > 0xFFFFFFFF)
                return false;
            uint8_t b = 0;

            uint32_t buffer_password; 
            
            bufferPayload[b] = 0x04;
            bufferPayload[++b] = password % 256;
            bufferPayload[++b] = (password / 256) & 0xFF;
            bufferPayload[++b] = ((password / 256) >> 8) & 0xFF;
            bufferPayload[++b] = ((password / 256) >> 16) & 0xFF;
            
            PrepareFrameCommand(localId, 0xCD, bufferPayload, b + 1);
            SendPacket();

            buffer_password = bufferPayload[2] << 8;
            buffer_password += bufferPayload[1];
                
            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK)
            {
                if(command == 0xCD)
                {
                    localread();
                    
                    if(buffer_password == registered_password)
                        return true;
                }
            }
            
            return false;  
        }

        bool read_config_bps(bool ignore_cmd = false){
            uint8_t b = 0;

            if(!ignore_cmd){
                bufferPayload[b] = 0x00;
                bufferPayload[++b] = 0x01;
                bufferPayload[++b] = 0x00;

                PrepareFrameCommand(localId, 0xD6, bufferPayload, b + 1);
                SendPacket();
            }

            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK){
                if(command == 0xD6){
                    BW = bufferPayload[2];
                    SF = bufferPayload[3];
                    CR = bufferPayload[4];
                    return true;
                }
            }
            return false;
        }

        bool read_config_class(bool ignore_cmd = false){
            uint8_t b = 0;
            if(!ignore_cmd){
                bufferPayload[b] = 0x00;
                bufferPayload[++b] = 0xFF;
                bufferPayload[++b] = 0x00;
                bufferPayload[++b] = 0x00;

                PrepareFrameCommand(localId, 0xC1, bufferPayload, b + 1);
                SendPacket();
            }
            
            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK){
                if(command == 0xC1){
                    LoRa_class = bufferPayload[2];
                    LoRa_window = bufferPayload[3];
            
                    return true;
                }
            }
            return false;
        }

        bool config_bps(uint8_t bandwidth = BW500, uint8_t spreading_factor = SF_LoRa_7, uint8_t coding_rate = CR4_5){
            if(bandwidth < 0x00 || bandwidth > 0x02)
                return false;
            else if(spreading_factor < 0x00 || spreading_factor > 0x00 && spreading_factor < 0x07 || spreading_factor > 0x0C)
                return false;
            else if(coding_rate < 0x00 || coding_rate < 0x01 || coding_rate > 0x04)
                return false;

            uint8_t b = 0;

            bufferPayload[b] = 0x01;
            bufferPayload[++b] = 0x14;
            bufferPayload[++b] = bandwidth;
            bufferPayload[++b] = spreading_factor;
            bufferPayload[++b] = coding_rate;
            
            PrepareFrameCommand(localId, 0xD6, bufferPayload, b + 1);
            SendPacket();

            read_config_bps(true);

            if(BW == bandwidth && SF == spreading_factor && CR == coding_rate)
                return true;

            return false;
        }

        bool config_class(uint8_t lora_class = LoRa_CLASS_C, uint8_t lora_window = LoRa_WINDOW_5s){
            if(lora_class < 0x00 || lora_class != 0x00 && lora_class != 0x02)
                return false;
            else if(lora_window < 0x00 || lora_window > 0x02)
                return false;
            else if(debug_serial && lora_class == 0x02 && lora_window > 0x00){
                Serial.println("Apenas a classe A tem janela de tempo dormindo");
                return false;
            }
            
            uint8_t b = 0;

            bufferPayload[b] = 0x00;
            bufferPayload[++b] = lora_class;
            bufferPayload[++b] = lora_window;
            bufferPayload[++b] = 0x00;

            PrepareFrameCommand(localId, 0xC1, bufferPayload, b + 1);
            SendPacket();
            read_config_class(true);

            if(LoRa_class == lora_class && LoRa_window == lora_window)
                return true;
            
            return false;
        }    

        bool config_digital_gpio(uint8_t gpio, uint8_t pull, uint8_t inout, uint8_t logical_level){
            if(gpio < 0x00 || gpio > 0x07 || gpio == 0x05 || gpio == 0x06)
                return false;
            else if(pull < 0x00 || pull > 0x02)
                return false;
            else if(inout < 0x00 || inout == 0x02 || inout == 0x03 && gpio < 0x05 && inout == 0x03 && gpio > 0x06 || inout > 0x03)
                return false;
            else if(logical_level < 0x00 || logical_level > 0x03)
                return false;
            
            uint8_t b = 0;

            bufferPayload[b] = 0x02;
            bufferPayload[++b] = gpio;
            bufferPayload[++b] = pull;
            bufferPayload[++b] = inout;
            bufferPayload[++b] = logical_level;

            PrepareFrameCommand(localId, 0xC2, bufferPayload, b + 1);
            SendPacket();
            
            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK)
            {
                if(command == 0xC2)
                    return true;
            }

            return false;
        }

        bool config_analog_gpio(uint8_t gpio){
            if(gpio < 0x05 || gpio > 0x06)
                return false;
            
            uint8_t b = 0;
            bufferPayload[b] = 0x02;
            bufferPayload[++b] = gpio;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = LoRa_INOUT_ANALOG_INPUT;
            bufferPayload[++b] = 0x00;

            PrepareFrameCommand(localId, 0xC2, bufferPayload, b + 1);
            SendPacket();

            return true;
        }

        void get_gpio_status(int16_t id, uint8_t gpio){
            uint8_t b = 0;

            bufferPayload[b] = 0x00;
            bufferPayload[++b] = gpio;
            bufferPayload[++b] = 0x00;
            bufferPayload[++b] = 0x00;

            PrepareFrameCommand(id, 0xC2, bufferPayload, b + 1);
            SendPacket();
        }

        double read_gpio(int16_t id, uint8_t gpio, bool analog = false){
            get_gpio_status(id, gpio);
            if(ReceivePacketCommand(&localId, &command, bufferPayload, &payloadSize, 1000) == MESH_OK)
            {
                if(command == 0xC2){
                    if(analog){
                        uint8_t voltage = (uint16_t)bufferPayload[4] | ((uint16_t)(bufferPayload[3]&0x0F) << 8);
                        return float(voltage)*8.0586e-4;
                    }
                    else
                        return bufferPayload[4];
                }
            }
            return 0;
        } 

        bool write_gpio(int16_t id, uint8_t gpio, uint8_t logical_level){
            if(gpio < 0x00 || gpio > 0x07)
                return false;
            else if(logical_level < 0x00 || logical_level > 0x03)
                return false;

            uint8_t b = 0;

            bufferPayload[b] = 0x01;
            bufferPayload[++b] = gpio;
            bufferPayload[++b] = logical_level;
            bufferPayload[++b] = 0x00;

            PrepareFrameCommand(id, 0xC2, bufferPayload, b + 1);
            SendPacket();
            
            return true;
        }
        
        void printHex(uint8_t* num, uint8_t tam){
            char hexCar[4];
            uint8_t index;
            
            for(index = 0; index < tam - 1; index++)
            {
                sprintf(hexCar, "%02X", num[index]);
                Serial.print(hexCar);
            }
            
            sprintf(hexCar, "%02X", num[index]);
            Serial.println(hexCar);  
        }
};
#endif

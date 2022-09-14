/*-------------------------------------------------------------------------------------
  Este código já faz uma autoconfiguração do módulo, então você não precisa
  fazer manualmente. Caso queria saber mais sobre esse módulo, acesse o link
  do nosso post.  

  Post completo : https://elcereza.com/lora-mesh-da-radioenge-tutorial-completo
  Autor : Gustavo Cereza
  Revisado e editado : Marco Chiodi (Diretor de P&D da Radioenge)
  Disponibilizado por : Elcereza
  Redes : @ellcereza
  Canal : https://t.me/elcereza
-------------------------------------------------------------------------------------*/

#include "LoRaMESH.h"
#include <SoftwareSerial.h>

SoftwareSerial SerialCommand(3, 4);
LoRaMESH lora(&SerialCommand);

uint8_t ID = 1;

void setup() {
  Serial.begin(115200);
  delay(2000);
  SerialCommand.begin(9600);
  lora.begin(true);
  pinMode(13, INPUT_PULLUP);

  if(lora.localId != ID)
  {
    if(!lora.setnetworkId(ID)){
      Serial.println("Erro ao definir o novo ID");
      while(1);
    }

    Serial.println("ID configurado com sucesso!");

    if(!lora.config_bps(BW500, SF_LoRa_7, CR4_5)){
      Serial.println("Erro ao configurar bps");
      while(1);
    }

    Serial.println("Parametros LoRa configurados com sucesso!");
    
    if(!lora.config_class(LoRa_CLASS_C, LoRa_WINDOW_15s)){
      Serial.println("Erro ao configurar a classe");
      while(1);
    }

    Serial.println("Modo de operacao configurado com sucesso!");

    if(!lora.setpassword(123)){
      Serial.println("Erro ao gravar a senha ou a senha gravada não condiz com a senha definida");
      while(1);
    }

    Serial.println("Senha configurada com sucesso!");
  }

  if(ID == 1)
    lora.config_digital_gpio(LoRa_GPIO0, LoRa_NOT_PULL, LoRa_INOUT_DIGITAL_OUTPUT, LoRa_LOGICAL_LEVEL_LOW);

  Serial.println("LocalID: " + String(lora.localId));
  Serial.println("UniqueID: " + String(lora.localUniqueId));
  Serial.println("Pass <= 65535: " + String(lora.registered_password));
}

void loop() {
  if(ID == 0 && digitalRead(13) == 0){
    static bool button_status = false;
    if(button_status)
      lora.write_gpio(1, LoRa_GPIO0, LoRa_LOGICAL_LEVEL_HIGH);
    else
      lora.write_gpio(1, LoRa_GPIO0, LoRa_LOGICAL_LEVEL_LOW);
    delay(1000);
  }
}

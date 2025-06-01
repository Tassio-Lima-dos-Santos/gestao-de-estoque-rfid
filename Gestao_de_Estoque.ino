#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 5

// Instanciando o objeto
MFRC522 rfid(SS_PIN, RST_PIN);

// Variáveis globais
MFRC522::StatusCode status;

// Declaração de funções úteis
void dump_byte_array(byte *buffer, byte bufferSize);
// void dump_string(byte *buffer, byte bufferSize); Ao invés de criar uma função inútil que nem um otário, só use Serial.write(byte *buffer, int size)

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // iniciando o módulo MFRC522
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak); // Salvando o tipo da Tag
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // print NUID in Serial Monitor in the hex format
      Serial.print("UID:");
      dump_byte_array(rfid.uid.uidByte, (byte)rfid.uid.size);
      /*for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }*/
      Serial.println();

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

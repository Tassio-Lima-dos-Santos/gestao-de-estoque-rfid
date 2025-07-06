#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 5

// Instanciando o objeto
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Variáveis globais
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;
byte block = 0;
byte sector = 0;

// Declaração de funções úteis
void dump_byte_array(byte *buffer, byte bufferSize);
// void dump_string(byte *buffer, byte bufferSize); Ao invés de criar uma função inútil que nem um otário, só use Serial.write(byte *buffer, int size)
void authenticate();
void write_on_block(byte *buffer, byte bufferSize);
void write_on_sector(byte *buffer, byte bufferSize);


void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin(); // init SPI bus
  mfrc522.PCD_Init(); // iniciando o módulo MFRC522

  // Setando a chave como a chave padrão FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) { // new tag is available
    if (mfrc522.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); // Salvando o tipo da Tag
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(mfrc522.PICC_GetTypeName(piccType));

      // print NUID in Serial Monitor in the hex format
      Serial.print("UID:");
      dump_byte_array(mfrc522.uid.uidByte, (byte)mfrc522.uid.size);
      /*for (int i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }*/
      Serial.println();

      mfrc522.PICC_HaltA(); // halt PICC
      mfrc522.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void authenticate(){
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

void write_on_block(byte *buffer, byte bufferSize){
  authenticate();
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

void write_on_sector(byte *buffer, byte bufferSize){
  for(int i = 0; i < 3; i++){
    block = sector*4+i;
    write_on_block(buffer+i*16, 16);
  }
}

void read_block(byte *buffer, byte bufferSize){
  authenticate();
  status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
}

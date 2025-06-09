#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 5

// "Constantes" globais
#define secaoNome 1
#define secaoLote 2
#define secaoFornecedor 3

// Instanciando o objeto
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Variáveis globais
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;
byte block = 0;
byte sector = 0;
byte input[48];

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin(); // init SPI bus
  mfrc522.PCD_Init(); // iniciando o módulo MFRC522
  mfrc522.PCD_StopCrypto1();

  // Setando a chave como a chave padrão FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if(!(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() )) return;

  Serial.println(F("Digite o nome do material"));
  readSerial();
  sector = secaoNome;
  if(write_on_sector(input, 48)) return;

  Serial.println(F("Digite o lote do material"));
  readSerial();
  sector = secaoLote;
  if(write_on_sector(input, 48)) return;

  Serial.println(F("Digite o fornecedor do material"));
  readSerial();
  sector = secaoFornecedor;
  if(write_on_sector(input, 48)) return;

  Serial.println(F("Escrita concluída!"));
  Serial.println();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

int authenticate(){
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  return 0;
}

int write_on_block(byte *buffer, byte bufferSize){
  if(authenticate()) return 1;
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  return 0;
}

int write_on_sector(byte *buffer, byte bufferSize){
  for(int i = 0; i < 3; i++){
    block = sector*4+i;
    if(write_on_block(buffer+i*16, 16)){
      Serial.println("ERRO NA ESCRITA!!!");
      return 1;
    }
  }
  return 0;
}

int read_block(byte *buffer, byte bufferSize){
  if(authenticate()) return 1;
  status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  return 0;
}

int read_section(byte *buffer, byte bufferSize){
  for(int i = 0; i < 3; i++){
    block = sector*4+i;
    if(read_block(buffer+i*16, 18)){
      Serial.println("ERRO NA LEITURA!!!");
      return 1;
    }
  }
  return 0;
}

void bufferClean(){
  for(int i = 0; i < 48; i++){
    input[i] = 0;
  }
}

void readSerial(){
  bufferClean();
  while(Serial.readBytes(input, 48) < 1);
}

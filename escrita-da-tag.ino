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

// Declaração de funções úteis
void dump_byte_array(byte *buffer, byte bufferSize);
// void dump_string(byte *buffer, byte bufferSize); Ao invés de criar uma função inútil que nem um otário, só use Serial.write(byte *buffer, int size)
void authenticate();
void write_on_block(byte *buffer, byte bufferSize);
void write_on_sector(byte *buffer, byte bufferSize);
void read_block(byte *buffer, byte bufferSize);
void read_section(byte *buffer, byte bufferSize);
void bufferClean();

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
  write_on_sector(input, 48);

  Serial.println(F("Digite o lote do material"));
  readSerial();
  sector = secaoLote;
  write_on_sector(input, 48);

  Serial.println(F("Digite o fornecedor do material"));
  readSerial();
  sector = secaoFornecedor;
  write_on_sector(input, 48);

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

void authenticate(){
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

void write_on_block(byte *buffer, byte bufferSize){
  authenticate();
  status = mfrc522.MIFARE_Write(block, buffer, 16);
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
  }
}

void read_section(byte *buffer, byte bufferSize){
  for(int i = 0; i < 3; i++){
    block = sector*4+i;
    read_block(buffer+i*16, 18);
  }
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

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
  if(write_on_sector(input, 48)){
    quitComm();
    return;
  }

  Serial.println(F("Digite o lote do material"));
  readSerial();
  sector = secaoLote;
  if(write_on_sector(input, 48)){
    quitComm();
    return;
  }

  Serial.println(F("Digite o fornecedor do material"));
  readSerial();
  sector = secaoFornecedor;
  if(write_on_sector(input, 48)){
    quitComm();
    return;
  }

  Serial.println(F("Escrita concluída!"));
  Serial.println();

  quitComm();
}

void dump_byte_array(byte *buffer, byte bufferSize) { // Função para printar um buffer de bytes no monitor serial na forma hexadecimal.
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

int authenticate(){ // Função de autenticação do PCD com o PICC, é necessário chamar essa função antes de fazer qualquer operação no PICC. A função retorna 0 se der certo e 1 se der errado.
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  return 0;
}

int write_on_block(byte *buffer){ // Função para escrever o buffer passado no argumento no bloco endereçado pela variável global block. Retorna 0 se der certo e 1 se der errado.
  if(authenticate()) return 1;
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  return 0;
}

int write_on_sector(byte *buffer){ // Função para escrever o buffer passado no argumento no setor endereçado pela variável global sector. Retorna 0 se der certo e 1 se der errado.
  for(int i = 0; i < 3; i++){
    block = sector*4+i;
    if(write_on_block(buffer+i*16)){
      Serial.println(F("ERRO NA ESCRITA!!!"));
      return 1;
    }
  }
  return 0;
}

int read_block(byte *buffer, byte bufferSize){ // Função para ler o bloco endereçado pela variável global block e salvar no buffer passado no argumento. Retorna 0 se der certo e 1 se der errado.
  if(authenticate()) return 1;
  status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 1;
  }
  return 0;
}

int read_section(byte *buffer){ // Função para ler o setor endereçado pela variável global sector e salvar no buffer passado no argumento. Retorna 0 se der certo e 1 se der errado.
  for(int i = 0; i < 3; i++){
    block = sector*4+i;
    if(read_block(buffer+i*16, 18)){
      Serial.println(F("ERRO NA LEITURA!!!"));
      return 1;
    }
  }
  return 0;
}

int setValue_block(int value){
  if(authenticate()) return -1;
  status = mfrc522.MIFARE_SetValue(block, value);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_SetValue() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }
  status = mfrc522.MIFARE_GetValue(block, &value);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_GetValue() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  return value;
}

int readValue_block(){
  int32_t value;
  if(authenticate()) return -1;
  status = mfrc522.MIFARE_GetValue(block, &value);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_GetValue() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  return value;
}

int increment_block(int inc){ // Função para incrementar o argumento passado no bloco endereçado pela variável block. Retorna o valor salvo no bloco após o incremento se der certo e -1 se der errado.
  int32_t value;
  if(authenticate()) return -1;
  status = mfrc522.MIFARE_Increment(block, inc);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Increment() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  status = mfrc522.MIFARE_Transfer(block);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Transfer() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  status = mfrc522.MIFARE_GetValue(block, &value);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_GetValue() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  return value;
}

int decrement_block(int inc){ // Função para decrementar o argumento passado no bloco endereçado pela variável block. Retorna o valor salvo no bloco após o decremento se der certo e -1 se der errado.
  int32_t value;
  if(authenticate()) return -1;
  status = mfrc522.MIFARE_Decrement(block, inc);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Decrement() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  status = mfrc522.MIFARE_Transfer(block);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Transfer() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  status = mfrc522.MIFARE_GetValue(block, &value);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_GetValue() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return -1;
  }
  return value;
}

void bufferClean(){ // Função para limpar o array input.
  for(int i = 0; i < 48; i++){
    input[i] = 0;
  }
}

void readSerial(){
  bufferClean();
  while(Serial.readBytes(input, 48) < 1);
  slashNDelete();
}

void slashNDelete(){
  for(int i = 0; i < 48; i++){
    if(input[i] == '\n'){
      input[i] = '\0';
    }
  }
}

float readSerial_float(){ // Função que limpa o array input e salva os 48 primeiros bytes lidos no monitor serial no array input. A função espera até ler algo.
  float entrada;
  do{
    entrada = Serial.parseFloat();
  }while(entrada <= 0);
  return entrada;
}

void quitComm(){
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

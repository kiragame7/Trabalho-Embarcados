/*
 * TESTE 2 - LEITOR RFID RC522
 * Objetivo: ver se o leitor funciona e descobrir o numero (UID) de cada chaveiro.
 *
 * Ligacoes (7 fios):
 *   RC522 SDA  -> Arduino 10
 *   RC522 SCK  -> Arduino 13
 *   RC522 MOSI -> Arduino 11
 *   RC522 MISO -> Arduino 12
 *   RC522 IRQ  -> (nao ligar)
 *   RC522 GND  -> Arduino GND
 *   RC522 RST  -> Arduino 9
 *   RC522 3.3V -> Arduino 3.3V   (NUNCA no 5V!)
 *
 * Abra o Monitor Serial (9600) e encoste um chaveiro no leitor.
 */
#include <SPI.h>
#include <MFRC522.h>

MFRC522 leitor(10, 9);   // SDA/SS = 10, RST = 9

void setup() {
  Serial.begin(9600);
  SPI.begin();
  leitor.PCD_Init();
  delay(50);

  byte versao = leitor.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print(F("Versao do RC522: 0x"));
  Serial.println(versao, HEX);
  if (versao == 0x00 || versao == 0xFF) {
    Serial.println(F("ERRO: o leitor nao respondeu. Confira os fios e o 3.3V."));
  } else {
    Serial.println(F("Leitor OK! Encoste um chaveiro..."));
  }
}

void loop() {
  if (!leitor.PICC_IsNewCardPresent() || !leitor.PICC_ReadCardSerial()) return;

  Serial.print(F("UID ("));
  Serial.print(leitor.uid.size);
  Serial.print(F(" bytes): "));
  for (byte i = 0; i < leitor.uid.size; i++) {
    if (leitor.uid.uidByte[i] < 0x10) Serial.print('0');
    Serial.print(leitor.uid.uidByte[i], HEX);
    Serial.print(' ');
  }
  Serial.println();

  if (leitor.uid.size == 4) {
    Serial.print(F("  Formato para o codigo: {"));
    for (byte i = 0; i < 4; i++) {
      Serial.print(F("0x"));
      if (leitor.uid.uidByte[i] < 0x10) Serial.print('0');
      Serial.print(leitor.uid.uidByte[i], HEX);
      if (i < 3) Serial.print(F(", "));
    }
    Serial.println('}');
  } else {
    Serial.println(F("  ATENCAO: este cartao nao tem 4 bytes, o SmartAccess nao vai aceitar."));
  }

  leitor.PICC_HaltA();
  leitor.PCD_StopCrypto1();
  delay(1000);
}

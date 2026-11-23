/*
 * TESTE 1 - DISPLAY LCD I2C
 * Objetivo: ver se o display esta ligado certo.
 *
 * Ligacoes (4 fios):
 *   LCD GND -> Arduino GND
 *   LCD VCC -> Arduino 5V
 *   LCD SDA -> Arduino A4
 *   LCD SCL -> Arduino A5
 *
 * O programa procura o display sozinho e mostra o endereco no Monitor Serial (9600).
 * Se o display acender mas nao aparecer texto: girar o parafuso do
 * quadradinho AZUL atras do display com uma chave de fenda pequena.
 */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C* lcd = nullptr;

byte procurarDisplay() {
  for (byte endereco = 1; endereco < 127; endereco++) {
    Wire.beginTransmission(endereco);
    if (Wire.endTransmission() == 0) return endereco;
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  byte endereco = procurarDisplay();
  if (endereco == 0) {
    Serial.println(F("ERRO: nenhum display encontrado. Confira os 4 fios (GND, VCC, SDA, SCL)."));
    return;
  }
  Serial.print(F("Display encontrado no endereco 0x"));
  Serial.println(endereco, HEX);
  Serial.println(F("Anote esse endereco! (0x27 ou 0x3F normalmente)"));

  lcd = new LiquidCrystal_I2C(endereco, 16, 2);
  lcd->init();
  lcd->backlight();
  lcd->setCursor(0, 0);
  lcd->print("SmartAccess");
  lcd->setCursor(0, 1);
  lcd->print("Teste LCD OK!");
}

void loop() {
  if (lcd == nullptr) return;
  // contador para mostrar que o Arduino esta funcionando
  lcd->setCursor(14, 0);
  unsigned long segundos = (millis() / 1000) % 100;
  if (segundos < 10) lcd->print(' ');
  lcd->print(segundos);
  delay(200);
}

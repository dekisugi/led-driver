/*
Copyright 2021 Dekisugi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#include <SPI.h>

#define SPI_SS PB12

/*
  BluePillのSPI2を使用している。
  SPIが1系統しかないマイコンを使用する場合は、
  コード中の「SPI_2.」を「SPI.」に書き換え、
  真下のSPIClassの行は消す。
*/
SPIClass SPI_2(2);

#define BRI PB1 // 明るさ
#define CCT PA6 // 色温度

/*
  最大電流指定(だいたい)
  12ビットを超えると呪われる。
  Vout = Vref(3.3V) * LIMIT / 4096
  Iout = Vout / Rsense(1ohm)
*/
const int limit_a = 200; // WarmWhite
const int limit_b = 200; // CoolWhite

const byte dac_a = 0b00000000;
const byte dac_b = 0b10000000;
const byte dac_func = 0b00110000;

void setup() {
  Serial.begin(9600);

  SPI_2.begin();
  SPI_2.setBitOrder(MSBFIRST);
  SPI_2.setClockDivider(SPI_CLOCK_DIV128); // 仕様上はDIV4まで上げられる。
  SPI_2.setDataMode(SPI_MODE0);
  pinMode(SPI_SS, OUTPUT);

  pinMode(PB1, INPUT_ANALOG);
  pinMode(PA6, INPUT_ANALOG);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  int bri = analogRead(BRI);
  int cct = analogRead(CCT);

  int cur_a = bri * map(cct, 0, 4095, 4095, 0) / 4096;
  int cur_b = bri * cct / 4096;

  cur_a = currentMap(cur_a, limit_a);
  cur_b = currentMap(cur_b, limit_a);

  setDac(dac_a, cur_a);
  setDac(dac_b, cur_b);

  Serial.println();
  delay(100);
}

int currentMap(int pos, int limit) {
  // CS32F103C8のADCは12ビット
  pos = map(pos, 20, 4075, 0, limit);
  pos = constrain(pos, 0, limit);
  Serial.print(pos);
  Serial.print("; ");
  return(pos);
}

// DAC == MCP49x2
void setDac(byte sel_dac, unsigned int volt) {
  byte bit_m = sel_dac | dac_func | (volt >> 8);
  byte bit_l = volt & 0xFF;

  digitalWrite(SPI_SS, LOW);
  SPI_2.transfer(bit_m);
  SPI_2.transfer(bit_l);
  digitalWrite(SPI_SS, HIGH);

  Serial.print(bit_m, BIN);
  Serial.print(", ");
  Serial.print(bit_l, BIN);
  Serial.print("; ");
}

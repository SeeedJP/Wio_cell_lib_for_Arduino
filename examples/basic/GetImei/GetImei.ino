/*
 * Get IMEI of a SIM.
 */

#include "NectisCellular.h"

NectisCellular Nectis;


void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("");
  Serial.println("--- START ---------------------------------------------------");

  Serial.println("### I/O Initialize.");
  Nectis.Init();
  delay(100);
  Serial.println("### Power supply cellular ON.");
  Nectis.PowerSupplyCellular(true);
  delay(100);

  Nectis.Bg96Begin();
  Nectis.InitLteM();

  Serial.println("### Setup completed.");

  char imei[16];
  char imsi[16];
  char tel[16];

  // IMEIを取得してみよう
  if(Nectis.GetIMEI(imei, sizeof(imei)) > 0){
    Serial.print("imei=");
    Serial.println(imei);
  }

  // IMSIを取得してみよう
  if(Nectis.GetIMSI(imsi, sizeof(imsi)) > 0){
    Serial.print("imsi=");
    Serial.println(imsi);
  }

  // 電話番号を取得してみよう
  if(Nectis.GetPhoneNumber(tel, sizeof(tel)) > 0){
    Serial.print("tel=");
    Serial.println(tel);
  }

  Serial.flush();
  delay(1);
}

void loop() {
}

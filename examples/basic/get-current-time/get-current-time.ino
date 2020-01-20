/*
 * Get JST.
 */

#include "NectisCellular.h"

#define INTERVAL  (5000)

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
}

void loop() {
  Serial.println("### Get time.");

  //    Get the current time.
  struct tm currentTime;
  char currentTimeStr[64];

  Nectis.GetCurrentTime(&currentTime, true);
  strftime(currentTimeStr, sizeof(currentTimeStr), "%Y/%m/%d %H:%M:%S", &currentTime);

  Serial.print("JST:");
  Serial.println(currentTimeStr);
  Serial.flush();
  delay(1);

  delay(INTERVAL);
}

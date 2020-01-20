/*
 * Get RSSI.
 */

#include <NectisCellular.h>

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
  Serial.println("### Get RSSI.");
  int rssi = Nectis.GetReceivedSignalStrengthIndicator();

  Serial.print("RSSI:");
  Serial.print(rssi);
  Serial.println("");

  delay(INTERVAL);
}

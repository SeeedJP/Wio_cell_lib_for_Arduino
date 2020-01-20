/*
 * Post uptime when the Grove-button is pressed.
 */

#include <NectisCellular.h>

#define BUTTON_PIN  (GROVE_ANALOG_1_1)

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
  Serial.println("### Power supply ON.");
//  Make sure that the MODULE_PWR_PIN is set to HIGH.
  Nectis.PowerSupplyGrove(true);
  delay(100);

  Nectis.Bg96Begin();
  Nectis.InitLteM();

  Serial.println("### Setup completed.");
}


void loop() {
  char postData[64];
  memset(&postData, 0x00, sizeof(postData));

  sprintf(postData, "{\"uptime\":%lu}", millis() / 1000);
  Serial.println(postData);
  
  Nectis.PostDataViaUdp(postData, sizeof(postData)-1);

  delay(10000);
}

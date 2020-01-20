/*
 * Post battery level in every 1 minute.
 */

#include <NectisCellular.h>
#include <NectisMcu.h>

NectisCellular Nectis;
NectisMcu Mcu;

char postData[2];


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
  // Make sure that the MODULE_PWR_PIN is set to HIGH.
  Nectis.PowerSupplyGrove(true);
  delay(100);

  Nectis.Bg96Begin();
  Nectis.InitLteM();
  
  Serial.println("### Setup completed.");
}

void loop() {
  // Battery Level
  float lipoVoltageLevelMv = Mcu.ReadVbat();
  float lipoVoltageLevelPercentageFloat = Mcu.mvToPercent(lipoVoltageLevelMv);
  uint8_t lipoVoltageLevelPercentageInt = (unsigned int)(lipoVoltageLevelPercentageFloat + 0.5F);
  Serial.printf("Lipo battery: %u[%%}\n", lipoVoltageLevelPercentageInt);

  unsigned int sizeOfPostDataBatteryLevel = Mcu.GetDataDigits(lipoVoltageLevelPercentageInt);
  char *postDataBatteryLevel;
  char tmpPostDataBatteryLevel[sizeOfPostDataBatteryLevel];
  postDataBatteryLevel = Mcu.ConvertIntoBinary(tmpPostDataBatteryLevel, lipoVoltageLevelPercentageInt,
                                                  sizeOfPostDataBatteryLevel);

  // Create post data in binary.
  memset(&postData[0], 0x00, sizeof(postData));
  memcpy(&postData[0], postDataBatteryLevel, sizeOfPostDataBatteryLevel);

  Nectis.PostDataViaHttp(postData);

  delay(60000);
}

#include <NectisCellular.h>

#define WEBHOOK_URL       "http://unified.soracom.io"

NectisCellular Nectis;

void setup() {
  char data[100];
  int status;

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

  delay(3000);
  
  Serial.println("### Post.");
  sprintf(data, "{\"value1\":\"uptime %lu\"}", millis() / 1000);
  Serial.print("Post:");
  Serial.print(data);
  Serial.println("");

//  if (!Nectis.HttpPost(WEBHOOK_URL, data, &status)) {
//    Serial.println("### ERROR! ###");
//  }

  Nectis.PostDataViaHttp(data);
  
  Serial.print("Status:");
  Serial.println(status);
}

void loop() {
}

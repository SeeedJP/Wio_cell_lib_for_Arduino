#include "NectisCellular.h"

constexpr char URL[] = "https://httpbin.org/ip";

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


  char data[512];
  int status;

  Serial.printf("GET %s\n", URL);
  status = Nectis.HttpGet(URL, data, sizeof(data));
  
  Serial.print("RecvBytes=");
  Serial.println(status);

  if(status > 0)
  {
    data[status] = 0x0;
    Serial.println(data);
  }
  Serial.print("Status=");
  Serial.println(status);
  Serial.println();

  Nectis.Bg96TurnOff();
  Nectis.Bg96End();

  Serial.flush();
  delay(1);
}

void loop() {
}

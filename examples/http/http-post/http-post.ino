#include "NectisCellular.h"

// TCP_IPに比べ、ヘッダーのサイズが小さくなるため、少ないオーバーヘッドとなる。
// UDPでデータを送信したい場合は、UDPをアンコメントして、TCP_IPをコメントアウトする。
#define UDP
// TCP_IPに比べ、ヘッダーのサイズが小さくなるため、少ないオーバーヘッドとなる。
// UDPでデータを送信したい場合は、TCP_IPをアンコメントして、UDPをコメントアウトする。
#define TCP

constexpr char ENDPOINT_URL[] = "http://unified.soracom.io";

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


  char data[128];
  int status;

  memset(&data[0], 0x00, sizeof(data));

  Serial.println("### Post.");
  sprintf(data, "{\"uptime\": %lu}", millis() / 1000);
  Serial.printf("Post=%s\n\n", data);

#ifdef UDP
  Nectis.PostDataUsingUdp(data, strlen(data));
#endif  //UDP

  memset(&data[0], 0x00, sizeof(data));

  Serial.println("### Post.");
  sprintf(data, "{\"uptime\": %lu}", millis() / 1000);
  Serial.printf("Post=%s\n\n", data);

#ifdef TCP
  Nectis.PostDataUsingTcp(data, strlen(data));
#endif  //TCP
  
  Serial.printf("Status=%d\n\n", status);

  Nectis.Bg96TurnOff();
  Nectis.Bg96End();

  Serial.flush();
  delay(1);
}

void loop() {
}

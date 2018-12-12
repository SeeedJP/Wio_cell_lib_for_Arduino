#include <WioCellLibforArduino.h>

#define INTERVAL        (60000)
#define RECEIVE_TIMEOUT (10000)

WioCellular Wio;
  
void setup() {
  delay(200);

  SerialUSB.begin(115200);
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyCellular(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
#ifdef ARDUINO_WIO_LTE_M1NB1_BG96
  Wio.SetAccessTechnology(WioCellular::ACCESS_TECHNOLOGY_LTE_M1);
  Wio.SetSelectNetwork(WioCellular::SELECT_NETWORK_MODE_MANUAL_IMSI);
#endif
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  char data[100];
  
  SerialUSB.println("### Open.");
  int connectId;
  connectId = Wio.SocketOpen("funnel.soracom.io", 23080, WIO_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  SerialUSB.println("### Send.");
  sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }
  
  SerialUSB.println("### Receive.");
  int length;
  length = Wio.SocketReceive(connectId, data, sizeof (data), RECEIVE_TIMEOUT);
  if (length < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err_close;
  }
  if (length == 0) {
    SerialUSB.println("### RECEIVE TIMEOUT! ###");
    goto err_close;
  }
  SerialUSB.print("Receive:");
  SerialUSB.print(data);
  SerialUSB.println("");

err_close:
  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  
err:
  delay(INTERVAL);
}

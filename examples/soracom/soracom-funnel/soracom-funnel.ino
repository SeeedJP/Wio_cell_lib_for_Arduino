#include <WioCellLibforArduino.h>

#define INTERVAL        (60000)
#define RECEIVE_TIMEOUT (10000)

WioCellular Wio;

void setup() {
    delay(200);

    Serial.begin(115200);
    Serial.println("");
    Serial.println("--- START ---------------------------------------------------");

    Serial.println("### I/O Initialize.");
    Wio.Init();

    Serial.println("### Power supply ON.");
    Wio.PowerSupplyCellular(true);
    delay(500);

    Serial.println("### Turn on or reset.");
#ifdef ARDUINO_WIO_LTE_M1NB1_BG96
    Wio.SetAccessTechnology(WioCellular::ACCESS_TECHNOLOGY_LTE_M1);
    Wio.SetSelectNetwork(WioCellular::SELECT_NETWORK_MODE_MANUAL_IMSI);
#endif
    if (!Wio.TurnOnOrReset()) {
        Serial.println("### ERROR! ###");
        return;
    }

    Serial.println("### Connecting to \"soracom.io\".");
    if (!Wio.Activate("soracom.io", "sora", "sora")) {
        Serial.println("### ERROR! ###");
        return;
    }

    Serial.println("### Setup completed.");
}

void loop() {
    char data[100];

    Serial.println("### Open.");
    int connectId;
    connectId = Wio.SocketOpen("funnel.soracom.io", 23080, WIO_UDP);
    if (connectId < 0) {
        Serial.println("### ERROR! ###");
        goto err;
    }

    Serial.println("### Send.");
    sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
    Serial.print("Send:");
    Serial.print(data);
    Serial.println("");
    if (!Wio.SocketSend(connectId, data)) {
        Serial.println("### ERROR! ###");
        goto err_close;
    }

    Serial.println("### Receive.");
    int length;
    length = Wio.SocketReceive(connectId, data, sizeof(data), RECEIVE_TIMEOUT);
    if (length < 0) {
        Serial.println("### ERROR! ###");
        goto err_close;
    }
    if (length == 0) {
        Serial.println("### RECEIVE TIMEOUT! ###");
        goto err_close;
    }
    Serial.print("Receive:");
    Serial.print(data);
    Serial.println("");

    err_close:
    Serial.println("### Close.");
    if (!Wio.SocketClose(connectId)) {
        Serial.println("### ERROR! ###");
        goto err;
    }

    err:
    delay(INTERVAL);
}

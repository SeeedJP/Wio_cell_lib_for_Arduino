// ToDO: Change WioCellular to NectisCellular

#include <WioCellLibforArduino.h>
#include <WioCellularClient.h>
#include <PubSubClient.h>        // https://github.com/SeeedJP/pubsubclient
#include <stdio.h>

#define APN               "soracom.io"
#define USERNAME          "sora"
#define PASSWORD          "sora"

#define MQTT_SERVER_HOST  "hostname"
#define MQTT_SERVER_PORT  (1883)

#define ID                "WioCell"
#define OUT_TOPIC         "outTopic"
#define IN_TOPIC          "inTopic"

#define INTERVAL          (60000)

WioCellular Wio;
WioCellularClient WioClient(&Wio);
PubSubClient MqttClient;

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Subscribe:");
    for (int i = 0; i < (int) length; i++)
        Serial.print((char) payload[i]);
    Serial.println("");
}

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

    Serial.println("### Connecting to \"" APN "\".");
    if (!Wio.Activate(APN, USERNAME, PASSWORD)) {
        Serial.println("### ERROR! ###");
        return;
    }

    Serial.println("### Connecting to MQTT server \"" MQTT_SERVER_HOST "\"");
    MqttClient.setServer(MQTT_SERVER_HOST, MQTT_SERVER_PORT);
    MqttClient.setCallback(callback);
    MqttClient.setClient(WioClient);
    if (!MqttClient.connect(ID)) {
        Serial.println("### ERROR! ###");
        return;
    }
    MqttClient.subscribe(IN_TOPIC);

    Serial.println("### Setup completed.");
}

void loop() {
    char data[100];
    sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
    Serial.print("Publish:");
    Serial.print(data);
    Serial.println("");
    MqttClient.publish(OUT_TOPIC, data);

    unsigned long next = millis();
    while (millis() < next + INTERVAL) {
        MqttClient.loop();
    }
}


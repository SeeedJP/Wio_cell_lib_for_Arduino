// ToDO: Change WioCellular to NectisCellular

#include <WioCellLibforArduino.h>
#include <WioCellularClient.h>
#include <NectisCellular.h>
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
NectisCellular Nectis;
PubSubClient MqttClient;

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Subscribe:");
    for (int i = 0; i < (int) length; i++)
        Serial.print((char) payload[i]);
    Serial.println("");
}

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


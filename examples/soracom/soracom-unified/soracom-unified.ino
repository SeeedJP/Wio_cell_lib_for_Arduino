#include <WioCellLibforArduino.h>

#define INTERVAL        (60000)
#define RECEIVE_TIMEOUT (10000)

// uncomment following line to use Temperature & Humidity sensor
// #define SENSOR_PIN    (WIO_D38)

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

#ifdef SENSOR_PIN
    TemperatureAndHumidityBegin(SENSOR_PIN);
#endif // SENSOR_PIN

    Serial.println("### Setup completed.");
}

void loop() {
    char data[100];

#ifdef SENSOR_PIN
    float temp;
    float humi;

    if (!TemperatureAndHumidityRead(&temp, &humi)) {
      Serial.println("ERROR!");
      goto err;
    }

    Serial.print("Current humidity = ");
    Serial.print(humi);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(temp);
    Serial.println("C");

    sprintf(data,"{\"temp\":%.1f,\"humi\":%.1f}", temp, humi);
#else
    sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
#endif // SENSOR_PIN

    Serial.println("### Open.");
    int connectId;
    connectId = Wio.SocketOpen("uni.soracom.io", 23080, WIO_UDP);
    if (connectId < 0) {
        Serial.println("### ERROR! ###");
        goto err;
    }

    Serial.println("### Send.");
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

////////////////////////////////////////////////////////////////////////////////////////
//

#ifdef SENSOR_PIN

int TemperatureAndHumidityPin;

void TemperatureAndHumidityBegin(int pin)
{
  TemperatureAndHumidityPin = pin;
  DHT11Init(TemperatureAndHumidityPin);
}

bool TemperatureAndHumidityRead(float* temperature, float* humidity)
{
  byte data[5];

  DHT11Start(TemperatureAndHumidityPin);
  for (int i = 0; i < 5; i++) data[i] = DHT11ReadByte(TemperatureAndHumidityPin);
  DHT11Finish(TemperatureAndHumidityPin);

  if(!DHT11Check(data, sizeof (data))) return false;
  if (data[1] >= 10) return false;
  if (data[3] >= 10) return false;

  *humidity = (float)data[0] + (float)data[1] / 10.0f;
  *temperature = (float)data[2] + (float)data[3] / 10.0f;

  return true;
}

#endif // SENSOR_PIN

////////////////////////////////////////////////////////////////////////////////////////
//

#ifdef SENSOR_PIN

void DHT11Init(int pin)
{
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
}

void DHT11Start(int pin)
{
  // Host the start of signal
  digitalWrite(pin, LOW);
  delay(18);

  // Pulled up to wait for
  pinMode(pin, INPUT);
  while (!digitalRead(pin)) ;

  // Response signal
  while (digitalRead(pin)) ;

  // Pulled ready to output
  while (!digitalRead(pin)) ;
}

byte DHT11ReadByte(int pin)
{
  byte data = 0;

  for (int i = 0; i < 8; i++) {
    while (digitalRead(pin)) ;

    while (!digitalRead(pin)) ;
    unsigned long start = micros();

    while (digitalRead(pin)) ;
    unsigned long finish = micros();

    if ((unsigned long)(finish - start) > 50) data |= 1 << (7 - i);
  }

  return data;
}

void DHT11Finish(int pin)
{
  // Releases the bus
  while (!digitalRead(pin)) ;
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
}

bool DHT11Check(const byte* data, int dataSize)
{
  if (dataSize != 5) return false;

  byte sum = 0;
  for (int i = 0; i < dataSize - 1; i++) {
    sum += data[i];
  }

  return data[dataSize - 1] == sum;
}

#endif // SENSOR_PIN

////////////////////////////////////////////////////////////////////////////////////////

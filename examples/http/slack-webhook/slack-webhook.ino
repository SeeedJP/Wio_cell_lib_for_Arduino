#include <WioCellLibforArduino.h>

// uncomment following line to use 'I2C High Accuracy Temp&Humi Sensor (SHT35)'.
// and you must install 'GroveDriverPack' library.
//#define HAVE_SHT35

#define APN               "soracom.io"
#define USERNAME          "sora"
#define PASSWORD          "sora"

#define WEBHOOK_URL       "<YOUR WEBHOOK URL>"

#define INTERVAL          (60000)

WioCellular Wio;
#if defined(HAVE_SHT35)
#include <GroveDriverPack.h>
GroveBoard Board;
GroveTempHumiSHT35 TempHumi(&Board.I2C);
#endif

void setup() {
  delay(200);

  SerialUSB.begin(115200);
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyCellular(true);
#if defined(HAVE_SHT35)
    Wio.PowerSupplyGrove(true);
#endif
  delay(500);

#if defined(HAVE_SHT35)
  Board.I2C.Enable();
  if (!TempHumi.Init()) {
    SerialUSB.println("### Sensor not found.");
  }
#endif

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to \"" APN "\".");
  if (!Wio.Activate(APN, USERNAME, PASSWORD)) {
    SerialUSB.println("ERROR!");
    return;
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  char data[100];
  int status;

#if defined(HAVE_SHT35)
  if (SerialUSB.available() >= 1) {
    switch (SerialUSB.read()) {
    case 'H':
      SerialUSB.println("On heater.");
      TempHumi.SetHeater(true);
      break;
    case 'h':
      SerialUSB.println("Off heater.");
      TempHumi.SetHeater(false);
      break;
    }
  }

  TempHumi.Read();

  char tempStr[16], humiStr[16];
  dtostrf(TempHumi.Temperature, 0, 2, tempStr);
  dtostrf(TempHumi.Humidity, 0, 2, humiStr);
  sprintf(data, "{\"text\":\"Current temperature = %s C, humidity = %s %%\"}", tempStr, humiStr);
#else
  sprintf(data, "{\"text\":\"uptime %lu\"}", millis() / 1000);
#endif

  SerialUSB.println("### Post.");
  SerialUSB.print("Post:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.HttpPost(WEBHOOK_URL, data, &status)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  SerialUSB.print("Status:");
  SerialUSB.println(status);

err:
  SerialUSB.println("### Wait.");
  delay(INTERVAL);
}

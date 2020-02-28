#include <WioCellLibforArduino.h>

#define APN               "soracom.io"
#define USERNAME          "sora"
#define PASSWORD          "sora"

#define SLACK_URL         "https://slack.com/api/chat.postMessage"
#define TOKEN             "<YOUR ACCESS-TOKEN>"
#define CHANNEL           "<YOUR CHANNEL-ID>"

#define INTERVAL          (60000)

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

static bool slackPostMessage(int* status, const char* token, const char* channel, const char* text) {
  char* authorization = (char*)malloc(strlen(token) + 8);
  sprintf(authorization, "Bearer %s", token);

  WioCellularHttpHeader header;
  header["Accept"] = "*/*";
  header["User-Agent"] = "Wio 3G";
  header["Connection"] = "Keep-Alive";
  header["Content-Type"] = "application/json";
  header["Authorization"] = authorization;

  char* data = (char*)malloc(strlen(token) + strlen(channel) + strlen(text) + 36);
  sprintf(data, "{\"token\":\"%s\",\"channel\":\"%s\",\"text\":\"%s\"}", token, channel, text);

#if defined(WIO_DEBUG)
  SerialUSB.print("Post:");
	for (auto it = header.begin(); it != header.end(); it++) {
		SerialUSB.print(it->first.c_str());
		SerialUSB.print(": ");
		SerialUSB.println(it->second.c_str());
	}
  SerialUSB.print(data);
  SerialUSB.println("");
#endif

  bool ret = Wio.HttpPost(SLACK_URL, data, status, header);
  free(authorization);
  free(data);
  return ret;
}

void loop() {
  char data[100];

  SerialUSB.println("### Post.");
  sprintf(data, "uptime: %lus", millis() / 1000);
  SerialUSB.println(data);

  int status;
  if (!slackPostMessage(&status, TOKEN, CHANNEL, data)) {
    SerialUSB.println("slack.postMessage failed.");
    goto err;
  }
  SerialUSB.print("Status:");
  SerialUSB.println(status);

err:
  SerialUSB.println("### Wait.");
  delay(INTERVAL);
}
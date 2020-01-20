#include <NectisCellular.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

#include <ArduinoJson.h>

using namespace Adafruit_LittleFS_Namespace;

#define DOWNLOAD_FW_HOST        "http://harvest-files.soracom.io"
#define HARVEST_FILES_PATH      "ota-dfu"

const char* firmwareVersion = "6.1.2";
const char* fileName = "fw_ver_6.1.2.json";

NectisCellular Nectis;
File file(InternalFS);

int contentLength;


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

  char downloadFwUrl[256];
  sprintf(downloadFwUrl, "%s/%s/fw_ver_%s.json", DOWNLOAD_FW_HOST, HARVEST_FILES_PATH, firmwareVersion);
  Serial.println(downloadFwUrl);

  char firmwareFile[1024];  // レスポンスを格納できるだけの容量が必要！ 現在2000byteはOKで3000byteはNG
  Serial.print("GET ");
  Serial.println(downloadFwUrl);
  Serial.println("");

  contentLength = Nectis.HttpGet(downloadFwUrl, firmwareFile, sizeof(firmwareFile));

  Serial.print("RecvBytes:");
  Serial.println(contentLength);

  if(contentLength > 0) {
    firmwareFile[contentLength] = 0x0;
    Serial.println(firmwareFile);   //取得したF/Wダミーファイルをコンソールに表示
  }
  Serial.print("contentLength:");
  Serial.println(contentLength);

  delay(1000);

  // Initialize Internal File System
  InternalFS.begin();

  file.open(fileName, FILE_O_READ);

  if( file.open(fileName, FILE_O_WRITE) ) {
    Serial.println("OK");
    file.write(firmwareFile, contentLength);
    file.close();
  } else {
    Serial.println("Failed!");
  }

  Nectis.UploadFilesToBg96(fileName, contentLength);

  // Exit from data mode and enter into AT command mode.
  delay(2000);
  digitalWrite(MODULE_DTR_PIN, HIGH);
  delay(1000);
  digitalWrite(MODULE_DTR_PIN, LOW);
  delay(2000);

  Serial.println(fileName);

  Nectis.ListBg96UfsFileInfo();

  delay(1000);
  Nectis.Bg96TurnOff();
  Nectis.Bg96End();
}

void loop() {

}
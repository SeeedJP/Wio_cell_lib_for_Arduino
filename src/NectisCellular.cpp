#include "NectisCellular.h"

#include "Internal/Debug.h"
#include "Internal/StringBuilder.h"
#include "Internal/ArgumentParser.h"

#include "NectisCellularConfig.h"

#include "WioCellular.h"
#include "WioCellularHardware.h"

#include <string.h>
#include <limits.h>

#include <nrf.h>
#include <Uart.h>


#define INTERVAL                        (10000)
#define RECEIVE_TIMEOUT                 (10000)
#define CONNECT_ID_NUM                  (12)
#define POLLING_INTERVAL                (100)

#define APN                             "soracom.io"
#define USERNAME                        "sora"
#define PASSWORD                        "sora"

#define ENDPOINT_URL                    "http://unified.soracom.io"

#define HTTP_USER_AGENT                 "QUECTEL_MODULE"

#define RET_OK(val)                     (ReturnOk(val))
#define RET_ERR(val, err)               (ReturnError(__LINE__, val, err))
#define LINEAR_SCALE(val, inMin, inMax, outMin, outMax)    (((val) - (inMin)) / ((inMax) - (inMin)) * ((outMax) - (outMin)) + (outMin))


NectisCellular::NectisCellular() : _SerialAPI(&SerialUART), _AtSerial(&_SerialAPI, this) {
}


////////////////////////////////////////////////////////////////////////////////////////
// Helper functions

//static bool SplitUrl(const char *url, const char **host, int *hostLength, const char **uri, int *uriLength) {
//    if (strncmp(url, "http://", 7) == 0) {
//    *host = &url[7];
//    } else if (strncmp(url, "https://", 8) == 0) {
//    *host = &url[8];
//    } else {
//    return false;
//    }
//
//    const char *ptr;
//    for (ptr = *host; *ptr != '\0'; ptr++) {
//    if (*ptr == '/')
//        break;
//    }
//    *hostLength = ptr - *host;
//    *uri = ptr;
//    *uriLength = strlen(ptr);
//
//    return true;
//}


////////////////////////////////////////////////////////////////////////////////////////
// The same as WioCellular

bool NectisCellular::ReturnError(int lineNumber, bool value, NectisCellular::ErrorCodeType errorCode) {
    _LastErrorCode = errorCode;

    char str[100];
    sprintf(str, "%d", lineNumber);
    DEBUG_PRINT("ERROR! ");
    DEBUG_PRINTLN(str);

    return value;
}

int NectisCellular::ReturnError(int lineNumber, int value, NectisCellular::ErrorCodeType errorCode) {
    _LastErrorCode = errorCode;

    char str[100];
    sprintf(str, "%d", lineNumber);
    DEBUG_PRINT("ERROR! ");
    DEBUG_PRINTLN(str);

    return value;
}

bool NectisCellular::IsBusy() const {
    return digitalRead(MODULE_STATUS_PIN) ? false : true;
}

bool NectisCellular::IsRespond() {
#ifndef ARDUINO_ARCH_STM32
    auto writeTimeout = SerialUART.getWriteTimeout();
    SerialUART.setWriteTimeout(10);
#endif // ARDUINO_ARCH_STM32

    Stopwatch sw;
    sw.Restart();
    while (!_AtSerial.WriteCommandAndReadResponse("AT", "^OK$", 500, NULL)) {
        if (sw.ElapsedMilliseconds() >= 2000) {
#ifndef ARDUINO_ARCH_STM32
            SerialUART.setWriteTimeout(writeTimeout);
#endif // ARDUINO_ARCH_STM32
            return false;
        }
    }

#ifndef ARDUINO_ARCH_STM32
    SerialUART.setWriteTimeout(writeTimeout);
#endif // ARDUINO_ARCH_STM32
    return true;
}

bool NectisCellular::Reset() {
    digitalWrite(MODULE_RESET_PIN, HIGH);
    delay(200);
    digitalWrite(MODULE_RESET_PIN, LOW);
    delay(300);

    return true;
}

bool NectisCellular::TurnOn() {
    delay(100);
    digitalWrite(MODULE_PWRKEY_PIN, HIGH);
    delay(600);
    digitalWrite(MODULE_PWRKEY_PIN, LOW);

    return true;
}

bool NectisCellular::HttpSetUrl(const char *url) {
    StringBuilder str;
    if (!str.WriteFormat("AT+QHTTPURL=%d", strlen(url)))
        return false;
    _AtSerial.WriteCommand(str.GetString());
    if (!_AtSerial.ReadResponse("^CONNECT$", 500, NULL))
        return false;

    _AtSerial.WriteBinary((const byte *) url, strlen(url));
    if (!_AtSerial.ReadResponse("^OK$", 500, NULL))
        return false;

    return true;
}

NectisCellular::ErrorCodeType NectisCellular::GetLastError() const {
    return _LastErrorCode;
}

void NectisCellular::Init() {
    ////////////////////
    // Module
    
    // Power Supply
    pinMode(MODULE_PWR_PIN, OUTPUT);            digitalWrite(MODULE_PWR_PIN, LOW);
    // Turn On/Off
    pinMode(MODULE_PWRKEY_PIN, OUTPUT);         digitalWrite(MODULE_PWRKEY_PIN, LOW);
    pinMode(MODULE_RESET_PIN, OUTPUT);          digitalWrite(MODULE_RESET_PIN, LOW);
    // Status Indication
    pinMode(MODULE_STATUS_PIN, INPUT_PULLUP);
    // Main SerialUART Interface
    pinMode(MODULE_DTR_PIN, OUTPUT);            digitalWrite(MODULE_DTR_PIN, LOW);
    
    ////////////////////
    // Led
    pinMode(LED_RED, OUTPUT);                   digitalWrite(LED_RED, LOW);
    pinMode(LED_BLUE, OUTPUT);                  digitalWrite(LED_BLUE, LOW);
    ////////////////////
    
    // AD Converter
    pinMode(BATTERY_LEVEL_ENABLE_PIN, OUTPUT);
    
    // Grove
    pinMode(GROVE_VCCB_PIN, OUTPUT);            digitalWrite(GROVE_VCCB_PIN, LOW);

    // RTC
    pinMode(RTC_INTRB, INPUT_PULLUP);           digitalWrite(RTC_INTRB, HIGH);
    pinMode(RTC_I2C_SDA_PIN, OUTPUT);           digitalWrite(RTC_I2C_SDA_PIN, HIGH);
}

void NectisCellular::PowerSupplyCellular(bool on) {
    digitalWrite(MODULE_PWR_PIN, on ? HIGH : LOW);
    delay(200);
    digitalWrite(MODULE_PWRKEY_PIN, on ? HIGH : LOW);
    delay(600);
    digitalWrite(MODULE_PWRKEY_PIN, LOW);
}

void NectisCellular::PowerSupplyGrove(bool on) {
    digitalWrite(MODULE_PWR_PIN, on ? HIGH : LOW);
    delay(100);
    digitalWrite(GROVE_VCCB_PIN, on ? HIGH : LOW);
}

bool NectisCellular::TurnOnOrReset() {
    std::string response;
    ArgumentParser parser;

    if (IsRespond()) {
        DEBUG_PRINTLN("Reset()");
        if (!Reset())
            return RET_ERR(false, E_UNKNOWN);
    } else {
        DEBUG_PRINTLN("TurnOn()");
        if (!TurnOn())
            return RET_ERR(false, E_UNKNOWN);
    }

    Stopwatch sw;
    sw.Restart();
    while (!_AtSerial.WriteCommandAndReadResponse("AT", "^OK$", 500, NULL)) {
        DEBUG_PRINT(".");
        DEBUG_PRINTLN("TurnOnOrReset: WriteCommandAndReadResponse");

        delay(100);

        if (sw.ElapsedMilliseconds() >= 10000) {
            DEBUG_PRINTLN("ElapsedMilliseconds");
            return RET_ERR(false, E_UNKNOWN);
        }
    }
    DEBUG_PRINTLN("");

    if (!_AtSerial.WriteCommandAndReadResponse("ATE0", "^OK$", 500, NULL))
        return RET_ERR(false, E_UNKNOWN);
    _AtSerial.SetEcho(false);

//#ifndef ARDUINO_ARCH_STM32
//    if (!_AtSerial.WriteCommandAndReadResponse("AT+IFC=2,2", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
//#endif // ARDUINO_ARCH_STM32

#if defined NRF52840_XXAA
    switch (_AccessTechnology) {
        case ACCESS_TECHNOLOGY_NONE:
            break;
        case ACCESS_TECHNOLOGY_LTE_M1:
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"nwscanseq\",02,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"nwscanmode\",3,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"iotopmode\",0,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"band\",F,20000,0,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            break;
        case ACCESS_TECHNOLOGY_LTE_NB1:
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"nwscanseq\",03,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"nwscanmode\",3,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"iotopmode\",1,1", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            // ToDO: Set the right band for NB-IoT
            // if (!_AtSerial.WriteCommandAndReadResponse("AT+QCFG=\"nb1/bandprior\",12", "^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
            break;
        default:
            return RET_ERR(false, E_UNKNOWN);
    }
#endif // NRF52840_XXAA

#if defined NECTIS_DEBUG
    sw.Restart();
    bool cpinReady;

    while (true) {
        _AtSerial.WriteCommand("AT+CPIN?");
        cpinReady = false;

        while (true) {
            if (!_AtSerial.ReadResponse("^(OK|\\+CPIN: READY|\\+CME ERROR: .*)$", 500, &response)) return RET_ERR(false, E_UNKNOWN);
            if (response == "+CPIN: READY") {
            cpinReady = true;
            continue;
            }
            break;
        }
        if (response == "OK" && cpinReady) break;

        if (sw.ElapsedMilliseconds() >= 10000) return RET_ERR(false, E_UNKNOWN);
        delay(POLLING_INTERVAL);
    }
#endif

#if defined NRF52840_XXAA
    sw.Restart();
    while (true) {
        int status;

        _AtSerial.WriteCommand("AT+CEREG?");
        if (!_AtSerial.ReadResponse("^\\+CEREG: (.*)$", 500, &response)) return RET_ERR(false, E_UNKNOWN);
        parser.Parse(response.c_str());

        if (parser.Size() < 2) return RET_ERR(false, E_UNKNOWN);
        //resultCode = atoi(parser[0]);
        status = atoi(parser[1]);

        if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
        if (0 <= status && status <= 5 && status != 4) break;
        if (sw.ElapsedMilliseconds() >= 10000) return RET_ERR(false, E_UNKNOWN);

        delay(POLLING_INTERVAL);
    }
#endif

    return RET_OK(true);
}

bool NectisCellular::TurnOff() {
    return _Wio.TurnOff();
}

int NectisCellular::GetIMEI(char *imei, int imeiSize){
    return _Wio.GetIMEI(imei, imeiSize);
}

int NectisCellular::GetIMSI(char *imsi, int imsiSize){
    return _Wio.GetIMSI(imsi, imsiSize);
}

int NectisCellular::GetICCID(char *iccid, int iccidSize){
    return _Wio.GetICCID(iccid, iccidSize);
}

int NectisCellular::GetPhoneNumber(char *number, int numberSize){
    return _Wio.GetPhoneNumber(number, numberSize);
}

int NectisCellular::GetReceivedSignalStrength() {
    std::string response;
    ArgumentParser parser;

    _AtSerial.WriteCommand("AT+CSQ");
    if (!_AtSerial.ReadResponse("^\\+CSQ: (.*)$", 500, &response))
        return RET_ERR(INT_MIN, E_UNKNOWN);

    parser.Parse(response.c_str());
    if (parser.Size() != 2)
        return RET_ERR(INT_MIN, E_UNKNOWN);
    int rssi = atoi(parser[0]);

    if (!_AtSerial.ReadResponse("^OK$", 500, NULL))
        return RET_ERR(INT_MIN, E_UNKNOWN);

    if (rssi == INT_MIN) {
        Serial.println("### ERROR! ###");
        delay(5000);
    }

    if (rssi == 0)
        return RET_OK(-113);
    else if (rssi == 1)
        return RET_OK(-111);
    else if (2 <= rssi && rssi <= 30)
        return RET_OK((int)LINEAR_SCALE((double)rssi, 2, 30, -109, -53));
    else if (rssi == 31)
        return RET_OK(-51);
    else if (rssi == 99)
        return RET_OK(-999);

    return RET_OK(-999);
}

bool NectisCellular::GetTime(struct tm *tim) {
    return _Wio.GetTime(tim);
}

#if defined NRF52840_XXAA
void NectisCellular::SetAccessTechnology(AccessTechnologyType technology) {
    _AccessTechnology = technology;
}
#endif // NRF52840_XXAA

void NectisCellular::SetSelectNetwork(SelectNetworkModeType mode, const char *plmn) {
    _SelectNetworkMode = mode;
    _SelectNetworkPLMN = plmn;
}

bool NectisCellular::WaitForCSRegistration(long timeout) {
    return _Wio.WaitForCSRegistration(timeout);
}

bool NectisCellular::WaitForPSRegistration(long timeout) {
    return _Wio.WaitForPSRegistration(timeout);
}

bool NectisCellular::Activate(const char *accessPointName, const char *userName, const char *password, long waitForRegistTimeout) {
    std::string response;
    ArgumentParser parser;
    Stopwatch sw;

    if (!WaitForPSRegistration(0)) {
//        StringBuilder str_apn;
//        if (!str_apn.WriteFormat("AT+CGDCONT=1, \"IP\", \"%s\"", accessPointName))
//            return RET_ERR(false, E_UNKNOWN);
//        if (!_AtSerial.WriteCommandAndReadResponse(str_apn.GetString(), "^OK$", 500, NULL))
//            return RET_ERR(false, E_UNKNOWN);

        StringBuilder str;
        if (!str.WriteFormat("AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",3", accessPointName, userName, password))
            return RET_ERR(false, E_UNKNOWN);
        if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", 500, NULL))
            return RET_ERR(false, E_UNKNOWN);

        sw.Restart();

        switch (_SelectNetworkMode) {
            case SELECT_NETWORK_MODE_NONE:
                break;
            case SELECT_NETWORK_MODE_AUTOMATIC:
                if (!_AtSerial.WriteCommandAndReadResponse("AT+COPS=0", "^OK$", waitForRegistTimeout, NULL))
                    return RET_ERR(false, E_UNKNOWN);
                break;
            case SELECT_NETWORK_MODE_MANUAL_IMSI: {
                char imsi[15 + 1];
                if (GetIMSI(imsi, sizeof(imsi)) < 0)
                    return RET_ERR(false, E_UNKNOWN);
                if (strlen(imsi) < 4)
                    return RET_ERR(false, E_UNKNOWN);
                StringBuilder str;
                if (!str.WriteFormat("AT+COPS=1,2,\"%.5s\"", imsi))
                    return RET_ERR(false, E_UNKNOWN);
                if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", waitForRegistTimeout, NULL))
                    return RET_ERR(false, E_UNKNOWN);
                break;
            }
            case SELECT_NETWORK_MODE_MANUAL: {
                if (_SelectNetworkPLMN.size() <= 0)
                    return RET_ERR(false, E_UNKNOWN);
                StringBuilder str;
                if (!str.WriteFormat("AT+COPS=1,2,\"%s\"", _SelectNetworkPLMN.c_str()))
                    return RET_ERR(false, E_UNKNOWN);
                if (!_AtSerial.WriteCommandAndReadResponse(str.GetString(), "^OK$", waitForRegistTimeout, NULL))
                    return RET_ERR(false, E_UNKNOWN);
                break;
            }
            default:
                return RET_ERR(false, E_UNKNOWN);
        }

        if (!WaitForPSRegistration(waitForRegistTimeout))
            return RET_ERR(false, E_UNKNOWN);

        // For debug.
#ifdef NECTIS_DEBUG
        char dbg[100];
        sprintf(dbg, "Elapsed time is %lu[msec.].", sw.ElapsedMilliseconds());
        DEBUG_PRINTLN(dbg);

        _AtSerial.WriteCommandAndReadResponse("AT+CREG?", "^OK$", 500, NULL);
        _AtSerial.WriteCommandAndReadResponse("AT+CGREG?", "^OK$", 500, NULL);
#if defined NRF52840_XXAA
        _AtSerial.WriteCommandAndReadResponse("AT+CEREG?", "^OK$", 500, NULL);
#endif // NRF52840_XXAA
#endif // NECTIS_DEBUG
    }

    sw.Restart();
    while (true) {
        _AtSerial.WriteCommand("AT+QIACT=1");
        if (!_AtSerial.ReadResponse("^(OK|ERROR)$", 150000, &response))
            return RET_ERR(false, E_UNKNOWN);
        if (response == "OK")
            break;
        if (!_AtSerial.WriteCommandAndReadResponse("AT+QIGETERROR", "^OK$", 500, NULL))
            return RET_ERR(false, E_UNKNOWN);
        if (sw.ElapsedMilliseconds() >= 150000)
            return RET_ERR(false, E_UNKNOWN);
        delay(POLLING_INTERVAL);
    }

    // For debug.
#ifdef NECTIS_DEBUG
    if (!_AtSerial.WriteCommandAndReadResponse("AT+QIACT?", "^OK$", 150000, NULL))
        return RET_ERR(false, E_UNKNOWN);
#endif // NECTIS_DEBUG

    return RET_OK(true);
}

bool NectisCellular::Deactivate() {
    return _Wio.Deactivate();
}

bool NectisCellular::GetDNSAddress(IPAddress *ip1, IPAddress *ip2) {
    return _Wio.GetDNSAddress(ip1, ip2);
}

bool NectisCellular::SetDNSAddress(const IPAddress &ip1) {
    return _Wio.SetDNSAddress(ip1);
}

bool NectisCellular::SetDNSAddress(const IPAddress &ip1, const IPAddress &ip2) {
    return _Wio.SetDNSAddress(ip1, ip2);
}

int NectisCellular::SocketOpen(const char *host, int port, SocketType type){
    return _Wio.SocketOpen(host, port, (WioCellular::SocketType)type);
}

bool NectisCellular::SocketSend(int connectId, const byte *data, int dataSize){
    return _Wio.SocketSend(connectId, data, dataSize);
}

bool NectisCellular::SocketSend(int connectId, const char *data){
    return _Wio.SocketSend(connectId, data);
}

int NectisCellular::SocketReceive(int connectId, byte *data, int dataSize){
    return _Wio.SocketReceive(connectId, data, dataSize);
}

int NectisCellular::SocketReceive(int connectId, byte *data, int dataSize, long timeout){
    return _Wio.SocketReceive(connectId, data, dataSize, timeout);
}

int NectisCellular::SocketReceive(int connectId, char *data, int dataSize){
    return _Wio.SocketReceive(connectId, data, dataSize);
}

int NectisCellular::SocketReceive(int connectId, char *data, int dataSize, long timeout){
    return _Wio.SocketReceive(connectId, data, dataSize,timeout);
}

bool NectisCellular::SocketClose(int connectId){
    return _Wio.SocketClose(connectId);
}

int NectisCellular::HttpGet(const char *url, char *data, int dataSize) {
    return _Wio.HttpGet(url, data, dataSize);
}

int NectisCellular::HttpGet(const char *url, char *data, int dataSize, const NectisCellularHttpHeader &header) {
    return _Wio.HttpGet(url,data,dataSize, header);
}

bool NectisCellular::HttpPost(const char *url, const char *data, const int dataSize, int *responseCode) {
    return _Wio.HttpPost(url, data, dataSize, responseCode);
}

bool NectisCellular::HttpPost(const char *url, const char *data, const int dataSize, int *responseCode, const NectisCellularHttpHeader &header) {
    return _Wio.HttpPost(url, data, dataSize, responseCode, header);
}

bool NectisCellular::HttpPost(const char *url, const byte *data, const int dataSize, int *responseCode) {
    return _Wio.HttpPost(url, data, dataSize, responseCode);
}

bool NectisCellular::HttpPost(const char *url, const byte *data, const int dataSize, int *responseCode, const NectisCellularHttpHeader &header) {
    return _Wio.HttpPost(url, data, dataSize, responseCode, header);
}

//TODO
// bool SendUSSD(const char *in, char *out, int outSize);

bool NectisCellular::HttpPost2(const char *url, const char *data, int *responseCode , char *recv_data, int recv_dataSize) {
    return _Wio.HttpPost2(url, data, responseCode, recv_data, recv_dataSize);
}

bool NectisCellular::HttpPost2(const char *url, const char *data, int *responseCode, char *recv_data, int recv_dataSize , const NectisCellularHttpHeader &header) {
    return _Wio.HttpPost2(url, data, responseCode, recv_data, recv_dataSize, header);
}


////////////////////////////////////////////////////////////////////////////////////////
// NetisCellular

void NectisCellular::SoftReset() {
    NVIC_SystemReset();
}

void NectisCellular::Bg96Begin() {
    // Initialize Uart between BL654 and BG96.
    Serial1.setPins(MODULE_UART_RX_PIN, MODULE_UART_TX_PIN, MODULE_RTS_PIN, MODULE_CTS_PIN);
    Serial1.begin(115200);

    delay(200);
}

void NectisCellular::Bg96End() {
    Serial1.end();
}

bool NectisCellular::Bg96TurnOff() {
    return TurnOff();
}

void NectisCellular::InitLteM() {
#ifdef ARDUINO_WIO_LTE_M1NB1_BG96
    SetAccessTechnology(ACCESS_TECHNOLOGY_LTE_M1);
    SetSelectNetwork(SELECT_NETWORK_MODE_MANUAL_IMSI);
#endif

    Serial.println("### Turn on or reset.");
    if (!TurnOnOrReset()) {
        Serial.println("### ERROR!; TurnOnOrReset ###");
        return;
    }

    delay(100);
    Serial.println("### Connecting to \"" APN "\".");
    if (!Activate(APN, USERNAME, PASSWORD)) {
        Serial.println("### ERROR!; Activate ###");
        return;
    }
}

void NectisCellular::InitNbIoT() {
    SetAccessTechnology(ACCESS_TECHNOLOGY_LTE_NB1);
    SetSelectNetwork(SELECT_NETWORK_MODE_MANUAL_IMSI);

    Serial.println("### Turn on or reset.");
    if (!TurnOnOrReset()) {
        Serial.println("### ERROR!; TurnOnOrReset ###");
        return;
    }

    delay(100);
    Serial.println("### Connecting to \"" APN "\".");
    if (!Activate("mtc.gen", "mtc", "mtc")) {
        Serial.println("### ERROR!; Activate ###");
        return;
    }
}

int NectisCellular::GetReceivedSignalStrengthIndicator() {
    int rssi = GetReceivedSignalStrength();
    int rssi_count = 0;
    while (rssi == - 999) {
        rssi = GetReceivedSignalStrength();
        if (rssi_count == 10) {
            SoftReset();
        }
        rssi_count++;
        delay(1000);
    }

    return rssi;
}

bool NectisCellular::IsTimeGot(struct tm *tim, bool jst) {
    std::string response;

    // AT+QLTS=1 -> Acquire UTC
    // AT+QLTS=2 -> Acquire JST
    if (jst) {
      _AtSerial.WriteCommand("AT+QLTS=2");
    } else {
      _AtSerial.WriteCommand("AT+QLTS=1");
    }
    
    if (!_AtSerial.ReadResponse("^\\+QLTS: (.*)$", 500, &response)) return RET_ERR(false, E_UNKNOWN);
    if (!_AtSerial.ReadResponse("^OK$", 500, NULL)) return RET_ERR(false, E_UNKNOWN);
    
    if (strlen(response.c_str()) != 26) return RET_ERR(false, E_UNKNOWN);
    
    const char* parameter = response.c_str();

    if (parameter[0] != '"') return RET_ERR(false, E_UNKNOWN);
    if (parameter[5] != '/') return RET_ERR(false, E_UNKNOWN);
    if (parameter[8] != '/') return RET_ERR(false, E_UNKNOWN);
    if (parameter[11] != ',') return RET_ERR(false, E_UNKNOWN);
    if (parameter[14] != ':') return RET_ERR(false, E_UNKNOWN);
    if (parameter[17] != ':') return RET_ERR(false, E_UNKNOWN);
    if (parameter[23] != ',') return RET_ERR(false, E_UNKNOWN);
    if (parameter[25] != '"') return RET_ERR(false, E_UNKNOWN);
    
    tim->tm_year = atoi(&parameter[1]) - 1900;
    tim->tm_mon = atoi(&parameter[6]) - 1;
    tim->tm_mday = atoi(&parameter[9]);
    tim->tm_hour = atoi(&parameter[12]);
    tim->tm_min = atoi(&parameter[15]);
    tim->tm_sec = atoi(&parameter[18]);
    tim->tm_wday = 0;
    tim->tm_yday = 0;
    tim->tm_isdst = 0;
    
    // Update tm_wday and tm_yday
    mktime(tim);
    
    return RET_OK(true);
}

void NectisCellular::GetCurrentTime(struct tm *tim, bool jst) {
    // Get time in JST.
    while (!IsTimeGot(tim, jst)) {
        Serial.println("### ERROR! ###");
        delay(5000);
    }
}


void NectisCellular::PostDataViaTcp(byte *post_data, int data_size) {
    Serial.println("### Post.");
    Serial.println("Post binary data.");
    
    constexpr char HTTP_CONTENT_TYPE[] = "application/octet-stream";

    NectisCellularHttpHeader header;
    header["Accept"] = "*/*";
    header["User-Agent"] = HTTP_USER_AGENT;
    header["Connection"] = "Keep-Alive";
    header["Content-Type"] = HTTP_CONTENT_TYPE;
    
    int status;
    if (!HttpPost(ENDPOINT_URL, post_data, (const int)data_size, &status, header)) {
        Serial.println("### ERROR! ###");
        goto err;
    }
    Serial.print("Status:");
    Serial.println(status);

err:
    Serial.println("### Wait.");
    delay(INTERVAL);
}

void NectisCellular::PostDataViaTcp(char *post_data, int data_size) {
    Serial.println("### Post.");
    Serial.print("Post:");
    Serial.print(post_data);
    Serial.println("");
    
    constexpr char HTTP_CONTENT_TYPE[] = "application/json";

    NectisCellularHttpHeader header;
    header["Accept"] = "*/*";
    header["User-Agent"] = HTTP_USER_AGENT;
    header["Connection"] = "Keep-Alive";
    header["Content-Type"] = HTTP_CONTENT_TYPE;
    
    int status;
    if (!HttpPost(ENDPOINT_URL, post_data, (const int)data_size, &status, header)) {
        Serial.println("### ERROR! ###");
        goto err;
    }
    Serial.print("Status:");
    Serial.println(status);

err:
    Serial.println("### Wait.");
    delay(INTERVAL);
}

void NectisCellular::PostDataViaUdp(byte *post_data, int data_size) {
    Serial.println("### Open.");
    int connectId;
    connectId = SocketOpen("uni.soracom.io", 23080, NECTIS_UDP);
    if (connectId < 0) {
        Serial.println("### ERROR! ###");
        goto err;
    }
    
    Serial.println("### Send.");
    Serial.println("Send binary data.");
    if (!SocketSend(connectId, post_data, data_size)) {
        Serial.println("### ERROR! ###");
        goto err_close;
    }
    
    Serial.println("### Receive.");
    int length;
    length = SocketReceive(connectId, post_data, data_size, RECEIVE_TIMEOUT);
    if (length < 0) {
        Serial.println("### ERROR! ###");
        Serial.println(length);
        goto err_close;
    }
    if (length == 0) {
        Serial.println("### RECEIVE TIMEOUT! ###");
        goto err_close;
    }
    Serial.print("Receive:");
    Serial.print((char *)post_data);
    Serial.println("");

err_close:
    Serial.println("### Close.");
    if (!SocketClose(connectId)) {
        Serial.println("### ERROR! ###");
        goto err;
    }

err:
    delay(INTERVAL);
}

void NectisCellular::PostDataViaUdp(char *post_data, int data_size) {
    Serial.println("### Open.");
    int connectId;
    connectId = SocketOpen("uni.soracom.io", 23080, NECTIS_UDP);
    if (connectId < 0) {
        Serial.println("### ERROR! ###");
        goto err;
    }
    
    Serial.println("### Send.");
    Serial.print("Send:");
    Serial.print(post_data);
    Serial.println("");
    if (!SocketSend(connectId, post_data)) {
        Serial.println("### ERROR! ###");
        goto err_close;
    }
    
    Serial.println("### Receive.");
    int length;
    length = SocketReceive(connectId, post_data, data_size, RECEIVE_TIMEOUT);
    if (length < 0) {
        Serial.println("### ERROR! ###");
        Serial.println(length);
        goto err_close;
    }
    if (length == 0) {
        Serial.println("### RECEIVE TIMEOUT! ###");
        goto err_close;
    }
    Serial.print("Receive:");
    Serial.print(post_data);
    Serial.println("");

err_close:
    Serial.println("### Close.");
    if (!SocketClose(connectId)) {
        Serial.println("### ERROR! ###");
        goto err;
    }

err:
    delay(INTERVAL);
}

void NectisCellular::GetBg96UfsStorageSize() {
    // +QFLDS: 12883392,14483456
    // The freesize is 12883392 byte, the total_size is 14483456 byte.
    _AtSerial.WriteCommandAndReadResponse("AT+QFLDS=\"UFS\"", "^OK$", 500, NULL);
    _AtSerial.WriteCommandAndReadResponse("AT+QFUPL=?", "^OK$", 500, NULL);
}

void NectisCellular::ListBg96UfsFileInfo() {
    _AtSerial.WriteCommandAndReadResponse("AT+QFLST", "^OK$", 500, NULL);
}

bool NectisCellular::UploadFilesToBg96(const char* filename, unsigned int filesize) {
    StringBuilder str;

    Serial.println(filename);
    if (!str.WriteFormat("AT+QFUPL=\"%s\",%u,60000,1", filename, filesize))
        return RET_ERR(false, E_UNKNOWN);
    _AtSerial.WriteCommand(str.GetString());
    if (!_AtSerial.ReadResponse("CONNECT", 60000, NULL))
        return RET_ERR(false, E_UNKNOWN);

    return true;
}

//void NectisCellular::DeleteBg96UfsFiles() {
//    _AtSerial.WriteCommandAndReadResponse("AT+QFDEL=\"*\"", "^OK$", 500, NULL);
//}

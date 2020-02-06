#pragma once

#include "NectisCellularConfig.h"

#include <IPAddress.h>
#include "Internal/AtSerial.h"
#include "Internal/WioSK6812.h"
#include <time.h>
#include <string>
#include "NectisCellularHttpHeader.h"

#define WIO_TCP        (WioCellular::SOCKET_TCP)
#define WIO_UDP        (WioCellular::SOCKET_UDP)

class WioCellular {
public:
    enum ErrorCodeType {
        E_OK = 0,
        E_UNKNOWN,
    };
    
    enum SocketType {
        SOCKET_TCP,
        SOCKET_UDP,
    };
    
    enum AccessTechnologyType {
        ACCESS_TECHNOLOGY_NONE,
        ACCESS_TECHNOLOGY_LTE_M1,
        ACCESS_TECHNOLOGY_LTE_NB1,
    };
    
    enum SelectNetworkModeType {
        SELECT_NETWORK_MODE_NONE,
        SELECT_NETWORK_MODE_AUTOMATIC,
        SELECT_NETWORK_MODE_MANUAL_IMSI,
        SELECT_NETWORK_MODE_MANUAL,
    };
    
private:
    SerialAPI _SerialAPI;
    AtSerial _AtSerial;
    WioSK6812 _Led;
    ErrorCodeType _LastErrorCode;
    AccessTechnologyType _AccessTechnology;
    SelectNetworkModeType _SelectNetworkMode;
    std::string _SelectNetworkPLMN;
    
private:
    bool ReturnOk(bool value) {
        _LastErrorCode = E_OK;
        return value;
    }
    int ReturnOk(int value) {
        _LastErrorCode = E_OK;
        return value;
    }
    bool ReturnError(int lineNumber, bool value, ErrorCodeType errorCode);
    int ReturnError(int lineNumber, int value, ErrorCodeType errorCode);
    
    bool IsBusy() const;
    bool IsRespond();
    bool Reset();
    bool TurnOn();
    
    bool HttpSetUrl(const char *url);
    
public:
    //bool ReadResponseCallback(const char* response);	// Internal use only.
    
public:
    WioCellular();
    ErrorCodeType GetLastError() const;
    void Init();
    void PowerSupplyCellular(bool on);
    void PowerSupplyLed(bool on);
    void PowerSupplyGrove(bool on);
    void LedSetRGB(uint8_t red, uint8_t green, uint8_t blue);
    bool TurnOnOrReset();
    bool TurnOff();
    //bool Sleep();
    //bool Wakeup();
    
    int GetIMEI(char *imei, int imeiSize);
    int GetIMSI(char *imsi, int imsiSize);
    int GetICCID(char *iccid, int iccidSize);
    int GetPhoneNumber(char *number, int numberSize);
    int GetReceivedSignalStrength();
    bool GetTime(struct tm *tim);

#if defined NRF52840_XXAA
    void SetAccessTechnology(AccessTechnologyType technology);
#endif // NRF52840_XXAA
    void SetSelectNetwork(SelectNetworkModeType mode, const char *plmn = NULL);
    bool WaitForCSRegistration(long timeout = 120000);
    bool WaitForPSRegistration(long timeout = 120000);
    bool Activate(const char *accessPointName, const char *userName, const char *password,
                  long waitForRegistTimeout = 120000);
    bool Deactivate();

    //bool GetLocation(double* longitude, double* latitude);

    bool GetDNSAddress(IPAddress *ip1, IPAddress *ip2);
    bool SetDNSAddress(const IPAddress &ip1);
    bool SetDNSAddress(const IPAddress &ip1, const IPAddress &ip2);
    
    int SocketOpen(const char *host, int port, SocketType type);
    bool SocketSend(int connectId, const byte *data, int dataSize);
    bool SocketSend(int connectId, const char *data);
    int SocketReceive(int connectId, byte *data, int dataSize);
    int SocketReceive(int connectId, char *data, int dataSize);
    int SocketReceive(int connectId, byte *data, int dataSize, long timeout);
    int SocketReceive(int connectId, char *data, int dataSize, long timeout);
    bool SocketClose(int connectId);
    
    int HttpGet(const char *url, char *data, int dataSize);
    int HttpGet(const char *url, char *data, int dataSize, const NectisCellularHttpHeader &header);
    bool HttpPost(const char *url, const char *data, int *responseCode);
    bool HttpPost(const char *url, const char *data, int *responseCode, const NectisCellularHttpHeader &header);
    
    bool SendUSSD(const char *in, char *out, int outSize);
    
    // ToDo: Pull Request
    bool HttpPost2(const char *url, const char *data, int *responseCode, char *recv_data, int recv_dataSize);
    bool HttpPost2(const char *url, const char *data, int *responseCode, char *recv_data, int recv_dataSize, const NectisCellularHttpHeader &header);
};

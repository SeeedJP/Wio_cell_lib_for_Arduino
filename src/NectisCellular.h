#pragma once

#include "NectisCellularConfig.h"
#include <string>

#include "WioCellular.h"
#include "WioCellLibforArduino.h"
#include "IPAddress.h"
#include "NectisCellularHttpHeader.h"
#include "Internal/AtSerial.h"
#include "Internal/Debug.h"

#define NECTIS_TCP        (NectisCellular::SOCKET_TCP)
#define NECTIS_UDP        (NectisCellular::SOCKET_UDP)

class NectisCellular {
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
    WioCellular _Wio;
    SerialAPI _SerialAPI;
    AtSerial _AtSerial;
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
    NectisCellular();
    ErrorCodeType GetLastError() const;
    void Init();
    void PowerSupplyCellular(bool on);
    void PowerSupplyGrove(bool on);
    bool TurnOnOrReset();
    bool TurnOff();

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
    bool Activate(const char *accessPointName, const char *userName, const char *password, long waitForRegistTimeout = 120000);
    bool Deactivate();

    bool GetLocation(double* longitude, double* latitude);

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
    bool HttpPost(const char *url, const char *data, const int dataSize, int *responseCode);
    bool HttpPost(const char *url, const char *data, const int dataSize, int *responseCode, const NectisCellularHttpHeader &header);
    bool HttpPost(const char *url, const byte *data, const int dataSize, int *responseCode);
    bool HttpPost(const char *url, const byte *data, const int dataSize, int *responseCode, const NectisCellularHttpHeader &header);

    bool SendUSSD(const char *in, char *out, int outSize);

    bool HttpPost2(const char *url, const char *data, int *responseCode, char *recv_data, int recv_dataSize);
    bool HttpPost2(const char *url, const char *data, int *responseCode, char *recv_data, int recv_dataSize , const NectisCellularHttpHeader &header);

    void SoftReset();
    void Bg96Begin();
    void Bg96End();
    bool Bg96TurnOff();
    void InitLteM();
    void InitNbIoT();

    int GetReceivedSignalStrengthIndicator();
    bool IsTimeGot(struct tm *tim, bool jst);
    void GetCurrentTime(struct tm *tim, bool jst);
    
    void PostDataViaTcp(byte *post_data, int data_size);
    void PostDataViaTcp(char *post_data, int data_size);
    void PostDataViaUdp(byte *post_data, int data_size);
    void PostDataViaUdp(char *post_data, int data_size);

    void GetBg96UfsStorageSize();
    void ListBg96UfsFileInfo();
    bool UploadFilesToBg96(const char* filename, unsigned int filesize);
//    void DeleteBg96UfsFiles();
};
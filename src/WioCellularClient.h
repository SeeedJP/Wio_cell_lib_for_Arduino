#pragma once

#include "NectisCellularConfig.h"

#include "WioCellular.h"
#include <Client.h>
#include <queue>

class WioCellularClient : public Client {
    
    protected:
    WioCellular *_Wio;
    int _ConnectId;
    std::queue <byte> _ReceiveQueue;
    byte *_ReceiveBuffer;
    
    public:
    WioCellularClient(WioCellular *wio);
    virtual ~WioCellularClient();
    
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual size_t write(uint8_t data);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();
    
};

#pragma once

#include "SerialAPI.h"
#include "Stopwatch.h"
#include <string>

class WioCellular;
class NectisCellular;

class AtSerial {
    private:
    SerialAPI *_Serial;
    WioCellular *_Wio;
    NectisCellular *_Nectis;
    unsigned long _EchoOn;
    
    bool ReadResponseInternal(const char *pattern, unsigned long timeout, std::string *response, int responseMaxLength);
    
    public:
    AtSerial(SerialAPI *serial, WioCellular *wio);
    AtSerial(SerialAPI *serial, NectisCellular *nectis);
    
    void SetEcho(bool on);
    
    bool WaitForAvailable(Stopwatch *sw, unsigned long timeout) const;
    
    void WriteBinary(const byte *data, int dataSize);
    bool ReadBinary(byte *data, int dataSize, unsigned long timeout);
    
    void WriteCommand(const char *command);
    bool ReadResponse(const char *pattern, unsigned long timeout, std::string *capture);
    bool
    WriteCommandAndReadResponse(const char *command, const char *pattern, unsigned long timeout, std::string *capture);
    
    bool ReadResponseQHTTPREAD(char *data, int dataSize, unsigned long timeout);
    
};

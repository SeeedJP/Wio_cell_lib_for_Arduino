#include "../NectisCellularConfig.h"
#include "AtSerial.h"

#include "Debug.h"
#include "slre.901d42c/slre.h"
#include "../NectisCellular.h"
#include <string.h>

#define READ_BYTE_TIMEOUT    (10)
#define RESPONSE_MAX_LENGTH    (1024)

#define CHAR_CR (0x0d)
#define CHAR_LF (0x0a)

AtSerial::AtSerial(SerialAPI* serial, NectisCellular* nectis) : _Serial(serial), _Nectis(nectis), _EchoOn(true)
{
}
AtSerial::AtSerial(SerialAPI *serial, NectisCellular *nectis) : _Serial(serial), _Nectis(nectis), _EchoOn(true) {
}

void AtSerial::SetEcho(bool on) {
    _EchoOn = on;
}

bool AtSerial::WaitForAvailable(Stopwatch *sw, unsigned long timeout) const {
    while (!_Serial->Available()) {
        if (sw != NULL && sw->ElapsedMilliseconds() >= timeout) {
            DEBUG_PRINTLN("### TIMEOUT ###");
            return false;
        }
    }
    
    return true;
}

void AtSerial::WriteBinary(const byte *data, int dataSize) {
    DEBUG_PRINTLN("<- (binary)");
    
    for (int i = 0; i < dataSize; i++) {
        _Serial->Write(data[i]);
    }
}

bool AtSerial::ReadBinary(byte *data, int dataSize, unsigned long timeout) {
    Stopwatch sw;
    for (int i = 0; i < dataSize; i++) {
        sw.Restart();
        if (!WaitForAvailable(&sw, timeout))
            return false;
        
        data[i] = _Serial->Read();
    }
    
    DEBUG_PRINTLN("-> (binary)");
    
    return true;
}

void AtSerial::WriteCommand(const char *command) {
    DEBUG_PRINT("<- ");
    DEBUG_PRINTLN(command);
    
    while (*command != '\0') {
        _Serial->Write((byte) * command);
        command++;
    }
    _Serial->Write((byte)CHAR_CR);
}

bool AtSerial::ReadResponse(const char* pattern, unsigned long timeout, std::string* capture)
{
	const char* internalPattern = NULL;
	if (pattern[strlen(pattern) - 1] != '$') {
		internalPattern = pattern;
	}

	Stopwatch sw;
	sw.Restart();
	while (true) {
		if (!WaitForAvailable(&sw, timeout)) return false;

		std::string response;
		if (!ReadResponseInternal(internalPattern, _EchoOn ? timeout : READ_BYTE_TIMEOUT, &response, RESPONSE_MAX_LENGTH)) return false;

		//if (_Nectis->ReadResponseCallback(response.c_str())) {
		//	continue;
		//}

		slre_cap cap;
		cap.len = 0;
		if (slre_match(pattern, response.c_str(), response.size(), &cap, 1, 0) >= 1) {
			if (capture != NULL) {
				capture->resize(cap.len);
				memcpy(&(*capture)[0], cap.ptr, cap.len);
			}
			return true;
		}
	}
}

bool AtSerial::ReadResponse(const char *pattern, unsigned long timeout, std::string *capture) {
    const char *internalPattern = NULL;
    if (pattern[strlen(pattern) - 1] != '$') {
        internalPattern = pattern;
    }
    
    Stopwatch sw;
    sw.Restart();
    while (true) {
        if (!WaitForAvailable(&sw, timeout))
            return false;
        
        std::string response;
        if (!ReadResponseInternal(internalPattern, _EchoOn ? timeout : READ_BYTE_TIMEOUT, &response,
                                  RESPONSE_MAX_LENGTH))
            return false;
        
        //if (_Wio->ReadResponseCallback(response.c_str())) {
        //	continue;
        //}
        
        slre_cap cap;
        cap.len = 0;
        if (slre_match(pattern, response.c_str(), response.size(), &cap, 1, 0) >= 1) {
            if (capture != NULL) {
                capture->resize(cap.len);
                memcpy(&(*capture)[0], cap.ptr, cap.len);
            }
            return true;
        }
    }
}

bool AtSerial::WriteCommandAndReadResponse(const char *command, const char *pattern, unsigned long timeout,
                                           std::string *capture) {
    WriteCommand(command);
    DEBUG_PRINT("WriteCommandAndReadResponse: ");
    DEBUG_PRINTLN(command);
    return ReadResponse(pattern, timeout, capture);
}

bool AtSerial::ReadResponseQHTTPREAD(char *data, int dataSize, unsigned long timeout) {
    int contentLength = 0;
    
    Stopwatch sw;
    sw.Restart();
    while (true) {
        if (!WaitForAvailable(&sw, timeout))
            return false;
        
        std::string response;
        if (!ReadResponseInternal(NULL, 1000, &response, dataSize - 2 - 1))
            return false;
        if (response == "OK")
            break;
        
        if (contentLength + response.size() + 2 + 1 > (size_t) dataSize)
            return false;
        memcpy(&data[contentLength], response.c_str(), response.size());
        strcpy(&data[contentLength + response.size()], "\r\n");
        contentLength += response.size() + 2;
    }
    if (contentLength >= 2 && strcmp(&data[contentLength - 2], "\r\n") == 0)
        contentLength -= 2;
    data[contentLength] = '\0';
    
    return true;
}

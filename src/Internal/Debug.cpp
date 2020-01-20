#include "../NectisCellularConfig.h"
#include "Debug.h"

#ifdef NECTIS_DEBUG

void Debug::Print(const char *str) {
    Serial.print(str);
}

void Debug::Println(const char *str) {
    Print(str);
    Print("\r\n");
}

#endif // NECTIS_DEBUG

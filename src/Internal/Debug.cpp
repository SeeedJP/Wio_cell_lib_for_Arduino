#include "../NectisCellularConfig.h"
#include "Debug.h"

#include "../NectisCellularHardware.h"

#ifdef CELLULAR_DEBUG

void Debug::Print(const char* str)
{
	Serial.print(str);
}

void Debug::Println(const char *str) {
  Print(str);
  Print("\r\n");
}

#endif // CELLULAR_DEBUG

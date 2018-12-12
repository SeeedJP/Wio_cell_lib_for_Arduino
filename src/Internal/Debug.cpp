#include "../WioCellularConfig.h"
#include "Debug.h"

#include "../WioCellularHardware.h"

#ifdef WIO_DEBUG

void Debug::Print(const char* str)
{
	SerialUSB.print(str);
}

void Debug::Println(const char* str)
{
	Print(str);
	Print("\r\n");
}

#endif // WIO_DEBUG

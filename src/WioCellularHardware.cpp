#include "WioCellularHardware.h"

void HardwareSerial::setReadBufferSize(int size) {
    _RxBufferCapacity = size;
}

unsigned long HardwareSerial::getWriteTimeout() const {
    return _TxTimeout;
}

void HardwareSerial::setWriteTimeout(unsigned long timeout) {
    _TxTimeout = timeout;
}

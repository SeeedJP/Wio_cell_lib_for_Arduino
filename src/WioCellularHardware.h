#pragma once

#include "NectisCellularConfig.h"

#include "Arduino.h"
#include "HardwareSerial.h"

#include <Uart.h>

// See hardware > cami > nrf52 > cores > nRF5 > Uart.h
#define SerialUART Serial1

extern Uart SerialUART;

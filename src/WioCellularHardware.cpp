#include "WioCellularConfig.h"
#include "WioCellularHardware.h"

#ifdef ARDUINO_ARCH_STM32

HardwareSerial SerialUSB(DEBUG_UART_RX_PIN, DEBUG_UART_TX_PIN);
HardwareSerial SerialModule(MODULE_UART_RX_PIN, MODULE_UART_TX_PIN);

HardwareSerial& SerialUART = Serial;
TwoWire& WireI2C = Wire;

#else

HardwareSerial SerialUSB(DEBUG_UART_CORE, DEBUG_UART_TX_PIN, DEBUG_UART_RX_PIN);
HardwareSerial SerialModule(MODULE_UART_CORE, MODULE_UART_TX_PIN, MODULE_UART_RX_PIN, MODULE_CTS_PIN, MODULE_RTS_PIN);

HardwareSerial Serial(GROVE_UART_CORE, GROVE_UART_TX_PIN, GROVE_UART_RX_PIN);
HardwareSerial& SerialUART = Serial;
TwoWire WireI2C(GROVE_I2C_CORE, GROVE_I2C_SDA_PIN, GROVE_I2C_SCL_PIN);

#endif // ARDUINO_ARCH_STM32

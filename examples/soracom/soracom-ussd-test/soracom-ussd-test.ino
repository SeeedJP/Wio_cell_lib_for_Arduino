#include <WioCellLibforArduino.h>

WioCellular Wio;

void setup() {
    delay(200);
    
    Serial.begin(115200);
    Serial.println("");
    Serial.println("--- START ---------------------------------------------------");
    
    Serial.println("### I/O Initialize.");
    Wio.Init();
    
    Serial.println("### Power supply ON.");
    Wio.PowerSupplyCellular(true);
    delay(500);
    
    Serial.println("### Turn on or reset.");
    if (!Wio.TurnOnOrReset()) {
        Serial.println("### ERROR! ###");
        return;
    }
    
    Serial.println("### Registering location.");
    if (!Wio.WaitForCSRegistration()) {
        Serial.println("### ERROR! ###");
        return;
    }
    
    Serial.println("### Sending USSD.");
    const char *message = "*901001*123# ";  // Unified Endpoint
    //const char* message = "*901011*123#";  // Beam
    //const char* message = "*901021*123# ";  // Funnel
    //const char* message = "*901031*123#";  // Harvest
    char response[256];
    
    if (!Wio.SendUSSD(message, response, sizeof(response))) {
        Serial.println("### ERROR! ###");
        return;
    }
    
    Serial.print("### Received response: ");
    Serial.println(response);
}

void loop() {

}

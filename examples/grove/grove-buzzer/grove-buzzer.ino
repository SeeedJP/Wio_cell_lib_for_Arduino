#include <WioCellLibforArduino.h>

#define BUZZER_PIN      (WIO_D38)
#define BUZZER_ON_TIME  (100)
#define BUZZER_OFF_TIME (3000)

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  delay(500);
}

void loop() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(BUZZER_ON_TIME);

  digitalWrite(BUZZER_PIN, LOW);
  delay(BUZZER_OFF_TIME);
}


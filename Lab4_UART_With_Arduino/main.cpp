// Lab: Arduino counts button presses and replies to STM32 over UART
// Wiring:
//  - Button: D2 ----(button)---- GND   (uses internal pull-up)
//  - UART:   Arduino RX(D0)  <- STM32 TX (USART1_TX)
//           Arduino TX(D1)  -> STM32 RX (USART1_RX)  [use divider/level shift]

volatile unsigned long pressCount = 0;

const int BTN_PIN = 2;

// Simple debounce for interrupt (ms)
const unsigned long DEBOUNCE_MS = 50;
volatile unsigned long lastIsrTime = 0;

void isrButton() {
  unsigned long now = millis();
  if (now - lastIsrTime >= DEBOUNCE_MS) {
    pressCount++;
    lastIsrTime = now;
  }
}

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), isrButton, FALLING);

  Serial.begin(115200);   // MUST match STM32 USART1 baud
  // Optional: give serial a moment to settle
  delay(50);
}

void loop() {
  // Lab protocol:
  // STM32 sends '?' every 10 seconds after wake.
  // Arduino replies with the pressCount as a line of text.
  if (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '?') {
      // Copy volatile safely
      noInterrupts();
      unsigned long countCopy = pressCount;
      interrupts();

      Serial.println(countCopy);  // sends "123\r\n"
    }
    // If STM32 sends something else, ignore.
  }
}

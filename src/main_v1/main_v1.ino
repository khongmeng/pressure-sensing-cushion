// =========================
// CONFIG
// =========================
#define FSR_PIN 33        // ADC pin
#define LED_PIN 2         // ESP32 onboard LED

#define READ_DELAY 100    // ms
#define THRESHOLD 1500    // adjust after testing

// =========================
// GLOBALS
// =========================
int fsrValue = 0;

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  Serial.println("\n[INIT] FSR Test Starting...");
}

// =========================
// MAIN LOOP
// =========================
void loop() {
  readFSR();
  handleLED();
  printData();

  delay(READ_DELAY);
}

// =========================
// FUNCTIONS
// =========================
void readFSR() {
  fsrValue = analogRead(FSR_PIN);
}

void handleLED() {
  if (fsrValue > THRESHOLD) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void printData() {
  Serial.print("[FSR] Value: ");
  Serial.print(fsrValue);

  Serial.print(" | State: ");
  if (fsrValue > THRESHOLD) {
    Serial.println("PRESSED");
  } else {
    Serial.println("RELEASED");
  }
}
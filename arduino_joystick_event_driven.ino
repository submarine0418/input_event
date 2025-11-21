// ============================================================
// HW-504 Joystick → Serial WASD One-Shot Event Mode
// 只在方向被觸發時傳送 W1 / A1 / S1 / D1，不傳放開事件
// ============================================================

const int PIN_VRX = A0;  // X 軸
const int PIN_VRY = A1;  // Y 軸
const int PIN_LED = 13; // Visual feedback

const int CENTER = 512;
const int THRESHOLD = 300; // Increased threshold to avoid noise

bool w_active = false;
bool a_active = false;
bool s_active = false;
bool d_active = false;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  
  // Flash LED 3 times on startup to show we are alive
  for(int i=0; i<3; i++) {
    digitalWrite(PIN_LED, HIGH); delay(100);
    digitalWrite(PIN_LED, LOW);  delay(100);
  }
}

void loop() {
  int x = analogRead(PIN_VRX);
  int y = analogRead(PIN_VRY);

  // --- Logic for W (UP) ---
  // Adjust direction (> or <) based on your wiring
  bool w_now = (y > CENTER + THRESHOLD); 
  if (w_now != w_active) {
    sendEvent(w_now ? "W1" : "W0");
    w_active = w_now;
  }

  // --- Logic for S (DOWN) ---
  bool s_now = (y < CENTER - THRESHOLD);
  if (s_now != s_active) {
    sendEvent(s_now ? "S1" : "S0");
    s_active = s_now;
  }

  // --- Logic for A (LEFT) ---
  bool a_now = (x < CENTER - THRESHOLD);
  if (a_now != a_active) {
    sendEvent(a_now ? "A1" : "A0");
    a_active = a_now;
  }

  // --- Logic for D (RIGHT) ---
  bool d_now = (x > CENTER + THRESHOLD);
  if (d_now != d_active) {
    sendEvent(d_now ? "D1" : "D0");
    d_active = d_now;
  }

  delay(20); // Small delay for stability
}

void sendEvent(const char* msg) {
  Serial.println(msg);
  // Flash LED briefly to confirm transmission
  digitalWrite(PIN_LED, HIGH);
  delay(10);
  digitalWrite(PIN_LED, LOW);
}

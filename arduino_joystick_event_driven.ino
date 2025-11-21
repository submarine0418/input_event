// ============================================================
// HW-504 Joystick → Serial WASD One-Shot Event Mode
// 只在方向被觸發時傳送 W1 / A1 / S1 / D1，不傳放開事件
// ============================================================

const int PIN_VRX = A0;  // X 軸
const int PIN_VRY = A1;  // Y 軸

const int CENTER = 512;
const int THRESHOLD = 200;     // 超出中心 ±200 才算移動
const int SAMPLE_INTERVAL_MS = 100;

bool w_active = false;
bool a_active = false;
bool s_active = false;
bool d_active = false;

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Arduino WASD One-Shot Ready");
}

void loop() {
  int x = analogRead(PIN_VRX);
  int y = analogRead(PIN_VRY);

  bool w_now = (y > CENTER + THRESHOLD);
  bool s_now = (y < CENTER - THRESHOLD);
  bool d_now = (x > CENTER + THRESHOLD);
  bool a_now = (x < CENTER - THRESHOLD);


  if (d_now && !d_active) {
    Serial.println("D");
    d_active = true;
  } else if (!d_now && d_active) {
    d_active = false;
  }

  if (s_now && !s_active) {
    Serial.println("S");
    s_active = true;
  } else if (!s_now && s_active) {
    s_active = false;
  }

  if (a_now && !a_active) {
    Serial.println("A");
    a_active = true;
  } else if (!a_now && a_active) {
    a_active = false;
  }

  if (w_now && !w_active) {
    Serial.println("W");
    w_active = true;
  } else if (!w_now && w_active) {
    w_active = false;
  }

  delay(SAMPLE_INTERVAL_MS);
}

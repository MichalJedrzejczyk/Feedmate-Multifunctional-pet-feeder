#include <LiquidCrystal.h>
#include <Servo.h>

// =====================================================
// FeedMate - Multifunctional Pet Feeder
// Arduino Mega 2560 version
//
// Features:
// - Manual feeding
// - Automatic interval-based feeding
// - LCD menu interface
// - 4x4 keypad navigation
// - Servo-controlled food dispensing
// - Buzzer feedback
// - LCD sleep mode
//
// Planned future improvements:
// - ESP32-based hardware version
// - Wi-Fi connectivity
// - Mobile application for remote control and monitoring
// =====================================================


// =====================================================
// LCD CONFIGURATION
// =====================================================
// Change LCD_COLS to 16 if you are using a 16x2 display.
const byte LCD_COLS = 20;
const byte LCD_ROWS = 2;

// LCD pins: RS, EN, D4, D5, D6, D7
const int LCD_RS = 12;
const int LCD_EN = 11;
const int LCD_D4 = 5;
const int LCD_D5 = 4;
const int LCD_D6 = 3;
const int LCD_D7 = 2;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);


// =====================================================
// BUZZER CONFIGURATION
// =====================================================
const int BUZZER_PIN = 9;


// =====================================================
// SERVO CONFIGURATION
// =====================================================
Servo feederServo;

const byte SERVO_PIN = A7;

const int CLOSED_ANGLE = 180;
const int OPEN_ANGLE = 90;

const int SERVO_STEP_DELAY_MS = 15;
const unsigned long SERVO_SETTLE_MS = 150;


// =====================================================
// KEYPAD CONFIGURATION
// =====================================================
// 4x4 keypad connected to Arduino Mega analog pins A8-A15.
byte rowPins[4] = {A8, A9, A10, A11};
byte colPins[4] = {A12, A13, A14, A15};

// Key mapping based on the physical keypad wiring.
char keys[4][4] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};

bool lastKeyState[4][4];


// =====================================================
// SCREEN AND MENU STATE
// =====================================================
enum Screen {
  SCREEN_START,
  SCREEN_MENU,
  SCREEN_EDIT,
  SCREEN_ACTION,
  SCREEN_STATUS
};

enum EditField {
  EDIT_NONE,
  EDIT_OPEN_TIME,
  EDIT_AUTO_INTERVAL
};

Screen currentScreen = SCREEN_START;
EditField currentEditField = EDIT_NONE;

const int MENU_LENGTH = 5;
int menuIndex = 0;


// =====================================================
// DEVICE SETTINGS
// =====================================================
struct Settings {
  unsigned long openTimeMs = 1000;        // Servo opening time in milliseconds
  bool autoModeEnabled = false;           // Automatic feeding ON/OFF
  unsigned long autoIntervalSec = 3600;   // Automatic feeding interval in seconds
  bool soundEnabled = true;               // Buzzer sounds ON/OFF
};

Settings settings;


// =====================================================
// RUNTIME STATE
// =====================================================
String editBuffer = "";
bool editFresh = true;

unsigned long actionScreenUntil = 0;
unsigned long nextAutomaticFeedAt = 0;


// =====================================================
// LCD SLEEP MODE
// =====================================================
bool lcdEnabled = true;
unsigned long lastUserActionAt = 0;
const unsigned long LCD_SLEEP_TIMEOUT_MS = 15000;


// =====================================================
// UI TEXT AND SCROLLING
// =====================================================
String topText = "";
String bottomText = "";

String lastPrintedTop = "";
String lastPrintedBottom = "";

unsigned long lastScrollAt = 0;
const unsigned long SCROLL_INTERVAL_MS = 220;

byte topScrollOffset = 0;
byte bottomScrollOffset = 0;


// =====================================================
// LCD HELPERS
// =====================================================
void clearLcdRow(byte row) {
  lcd.setCursor(0, row);
  for (byte i = 0; i < LCD_COLS; i++) {
    lcd.print(' ');
  }
}

void printLcdRow(byte row, const String &text, String &lastPrintedText) {
  String output = text;

  if (output.length() > LCD_COLS) {
    output = output.substring(0, LCD_COLS);
  }

  // Avoid unnecessary LCD rewriting to reduce flickering.
  if (output == lastPrintedText) {
    return;
  }

  clearLcdRow(row);
  lcd.setCursor(0, row);
  lcd.print(output);

  lastPrintedText = output;
}

String createMarqueeText(const String &text, byte &offset) {
  if (text.length() <= LCD_COLS) {
    return text;
  }

  String paddedText = text + "   " + text;

  if (offset >= paddedText.length()) {
    offset = 0;
  }

  return paddedText.substring(offset, offset + LCD_COLS);
}

void resetUiScroll() {
  topScrollOffset = 0;
  bottomScrollOffset = 0;
  lastScrollAt = millis();

  lastPrintedTop = "";
  lastPrintedBottom = "";
}

void renderUi() {
  if (!lcdEnabled) {
    return;
  }

  bool shouldUpdateScroll = false;

  if (millis() - lastScrollAt >= SCROLL_INTERVAL_MS) {
    lastScrollAt = millis();
    shouldUpdateScroll = true;
  }

  if (topText.length() > LCD_COLS && shouldUpdateScroll) {
    topScrollOffset++;
  }

  if (bottomText.length() > LCD_COLS && shouldUpdateScroll) {
    bottomScrollOffset++;
  }

  String topLine = createMarqueeText(topText, topScrollOffset);
  String bottomLine = createMarqueeText(bottomText, bottomScrollOffset);

  printLcdRow(0, topLine, lastPrintedTop);
  printLcdRow(1, bottomLine, lastPrintedBottom);
}


// =====================================================
// TIME FORMATTER
// =====================================================
String formatInterval(unsigned long seconds) {
  if (seconds < 60) {
    return String(seconds) + "s";
  }

  unsigned long minutes = seconds / 60;
  unsigned long remainingSeconds = seconds % 60;

  if (minutes < 60) {
    if (remainingSeconds == 0) {
      return String(minutes) + "m";
    }

    return String(minutes) + "m" + String(remainingSeconds) + "s";
  }

  unsigned long hours = minutes / 60;
  minutes %= 60;

  if (minutes == 0) {
    return String(hours) + "h";
  }

  return String(hours) + "h" + String(minutes) + "m";
}


// =====================================================
// LCD SLEEP MODE
// =====================================================
void wakeLcd() {
  if (!lcdEnabled) {
    lcd.display();
    lcdEnabled = true;

    lastPrintedTop = "";
    lastPrintedBottom = "";
  }

  lastUserActionAt = millis();
}

void handleLcdSleep() {
  if (lcdEnabled && millis() - lastUserActionAt > LCD_SLEEP_TIMEOUT_MS) {
    lcd.noDisplay();
    lcdEnabled = false;
  }
}


// =====================================================
// SOUND FEEDBACK
// =====================================================
void playClickSound() {
  if (settings.soundEnabled) {
    tone(BUZZER_PIN, 1800, 25);
  }
}

void playConfirmSound() {
  if (!settings.soundEnabled) {
    return;
  }

  tone(BUZZER_PIN, 1200, 60);
  delay(80);

  tone(BUZZER_PIN, 1600, 80);
  delay(110);

  noTone(BUZZER_PIN);
}

void playBackSound() {
  if (!settings.soundEnabled) {
    return;
  }

  tone(BUZZER_PIN, 700, 90);
  delay(110);

  noTone(BUZZER_PIN);
}

void playFeedingMelody() {
  if (!settings.soundEnabled) {
    return;
  }

  tone(BUZZER_PIN, 988, 120);
  delay(140);

  tone(BUZZER_PIN, 1319, 120);
  delay(140);

  tone(BUZZER_PIN, 1568, 140);
  delay(170);

  tone(BUZZER_PIN, 1319, 120);
  delay(140);

  tone(BUZZER_PIN, 1760, 220);
  delay(260);

  noTone(BUZZER_PIN);
}


// =====================================================
// SERVO CONTROL
// =====================================================
void moveServoSmoothly(int fromAngle, int toAngle) {
  fromAngle = constrain(fromAngle, 0, 180);
  toAngle = constrain(toAngle, 0, 180);

  if (fromAngle < toAngle) {
    for (int angle = fromAngle; angle <= toAngle; angle++) {
      feederServo.write(angle);
      delay(SERVO_STEP_DELAY_MS);
    }
  } else {
    for (int angle = fromAngle; angle >= toAngle; angle--) {
      feederServo.write(angle);
      delay(SERVO_STEP_DELAY_MS);
    }
  }
}

void openAndCloseFeeder(unsigned long openTimeMs) {
  feederServo.write(CLOSED_ANGLE);
  delay(SERVO_SETTLE_MS);

  moveServoSmoothly(CLOSED_ANGLE, OPEN_ANGLE);
  delay(openTimeMs);

  moveServoSmoothly(OPEN_ANGLE, CLOSED_ANGLE);
  delay(SERVO_SETTLE_MS);
}


// =====================================================
// KEYPAD READING
// =====================================================
char readKeyOnce() {
  for (int row = 0; row < 4; row++) {
    digitalWrite(rowPins[row], LOW);

    for (int col = 0; col < 4; col++) {
      bool isPressed = digitalRead(colPins[col]) == LOW;

      if (isPressed && !lastKeyState[row][col]) {
        char key = keys[row][col];

        lastKeyState[row][col] = true;
        digitalWrite(rowPins[row], HIGH);

        delay(120); // Simple debounce delay
        return key;
      }

      lastKeyState[row][col] = isPressed;
    }

    digitalWrite(rowPins[row], HIGH);
  }

  return 0;
}


// =====================================================
// SCREEN LEGENDS
// =====================================================
String getLegendForScreen(Screen screen) {
  switch (screen) {
    case SCREEN_START:
      return "A:Feed  D:Menu  B:Time";

    case SCREEN_MENU:
      return "B/C:Move  A:OK  D:Back  #:Status";

    case SCREEN_EDIT:
      return "0-9:Input  *:Del  #:Clear  A:OK  D:Cancel";

    case SCREEN_STATUS:
      return "D:Back  A:Start";

    case SCREEN_ACTION:
      return "D:Start  A:Repeat";
  }

  return "";
}


// =====================================================
// MENU ITEMS
// =====================================================
String getMenuItemText(int index) {
  unsigned long openTimeSec = settings.openTimeMs / 1000UL;

  switch (index) {
    case 0:
      return "Feed now";

    case 1:
      return "Open time: " + String(openTimeSec) + "s";

    case 2:
      return String("AUTO: ") + (settings.autoModeEnabled ? "ON" : "OFF");

    case 3:
      return "AUTO every: " + formatInterval(settings.autoIntervalSec);

    case 4:
      return String("Sound: ") + (settings.soundEnabled ? "ON" : "OFF");
  }

  return "";
}


// =====================================================
// SCREEN DRAWING
// =====================================================
void setScreen(Screen screen, const String &mainText) {
  currentScreen = screen;

  topText = mainText;
  bottomText = getLegendForScreen(screen);

  resetUiScroll();
  renderUi();
}

void drawStartScreen() {
  setScreen(SCREEN_START, "FEED MATE");
}

void drawMenuScreen() {
  setScreen(SCREEN_MENU, getMenuItemText(menuIndex));
}

void drawStatusScreen() {
  unsigned long openTimeSec = settings.openTimeMs / 1000UL;

  String status = "Open: " + String(openTimeSec) + "s  ";

  if (settings.autoModeEnabled) {
    status += "AUTO:" + formatInterval(settings.autoIntervalSec);
  } else {
    status += "AUTO:OFF";
  }

  setScreen(SCREEN_STATUS, status);
}

void showActionScreen(const String &message, unsigned long durationMs) {
  setScreen(SCREEN_ACTION, message);
  actionScreenUntil = millis() + durationMs;
}


// =====================================================
// EDIT MODE
// =====================================================
void startEditMode(EditField field) {
  currentEditField = field;
  editFresh = true;

  if (field == EDIT_OPEN_TIME) {
    editBuffer = String(settings.openTimeMs / 1000UL);
    setScreen(SCREEN_EDIT, "Open time (s): " + editBuffer);
  } else if (field == EDIT_AUTO_INTERVAL) {
    editBuffer = String(settings.autoIntervalSec);
    setScreen(SCREEN_EDIT, "AUTO every (s): " + editBuffer);
  } else {
    editBuffer = "";
    setScreen(SCREEN_EDIT, "Input:");
  }
}

void refreshEditScreen() {
  if (currentEditField == EDIT_OPEN_TIME) {
    topText = "Open time (s): " + editBuffer;
  } else if (currentEditField == EDIT_AUTO_INTERVAL) {
    topText = "AUTO every (s): " + editBuffer;
  } else {
    topText = "Input: " + editBuffer;
  }

  resetUiScroll();
  renderUi();
}

void confirmEditValue() {
  if (editBuffer.length() == 0) {
    editBuffer = "0";
  }

  long value = editBuffer.toInt();

  if (currentEditField == EDIT_OPEN_TIME) {
    value = constrain(value, 0, 60);
    settings.openTimeMs = (unsigned long)value * 1000UL;
  }

  if (currentEditField == EDIT_AUTO_INTERVAL) {
    value = constrain(value, 1, 86400);
    settings.autoIntervalSec = (unsigned long)value;

    if (settings.autoModeEnabled) {
      scheduleNextAutomaticFeed();
    }
  }

  drawMenuScreen();
}


// =====================================================
// FEEDING LOGIC
// =====================================================
void performFeeding(unsigned long openTimeMs, bool automaticMode) {
  playFeedingMelody();

  if (automaticMode) {
    showActionScreen("AUTO: feeding...", 700);
  } else {
    showActionScreen("Feeding...", 700);
  }

  openAndCloseFeeder(openTimeMs);

  showActionScreen("Done.", 1500);
}


// =====================================================
// AUTOMATIC FEEDING
// =====================================================
void scheduleNextAutomaticFeed() {
  nextAutomaticFeedAt = millis() + (settings.autoIntervalSec * 1000UL);
}

void handleAutomaticFeeding() {
  if (!settings.autoModeEnabled) {
    return;
  }

  if ((long)(millis() - nextAutomaticFeedAt) >= 0) {
    wakeLcd();

    performFeeding(settings.openTimeMs, true);
    scheduleNextAutomaticFeed();

    drawStartScreen();
  }
}


// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();

  for (int row = 0; row < 4; row++) {
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);
  }

  for (int col = 0; col < 4; col++) {
    pinMode(colPins[col], INPUT_PULLUP);
  }

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      lastKeyState[row][col] = false;
    }
  }

  feederServo.attach(SERVO_PIN, 600, 2400);
  feederServo.write(CLOSED_ANGLE);

  lastUserActionAt = millis();

  drawStartScreen();
  scheduleNextAutomaticFeed();
}


// =====================================================
// MAIN LOOP
// =====================================================
void loop() {
  handleLcdSleep();
  renderUi();
  handleAutomaticFeeding();

  char key = readKeyOnce();

  if (!key) {
    if (currentScreen == SCREEN_ACTION && millis() > actionScreenUntil) {
      drawStartScreen();
    }

    return;
  }

  wakeLcd();
  playClickSound();

  // =====================================================
  // START SCREEN CONTROLS
  // =====================================================
  if (currentScreen == SCREEN_START) {
    if (key == 'A') {
      playConfirmSound();
      performFeeding(settings.openTimeMs, false);
      return;
    }

    if (key == 'D') {
      playBackSound();
      drawMenuScreen();
      return;
    }

    if (key == 'B') {
      playConfirmSound();
      startEditMode(EDIT_OPEN_TIME);
      return;
    }

    return;
  }


  // =====================================================
  // MENU SCREEN CONTROLS
  // =====================================================
  if (currentScreen == SCREEN_MENU) {
    if (key == 'B') {
      menuIndex = (menuIndex - 1 + MENU_LENGTH) % MENU_LENGTH;
      drawMenuScreen();
      return;
    }

    if (key == 'C') {
      menuIndex = (menuIndex + 1) % MENU_LENGTH;
      drawMenuScreen();
      return;
    }

    if (key == '#') {
      playConfirmSound();
      drawStatusScreen();
      return;
    }

    if (key == 'D') {
      playBackSound();
      drawStartScreen();
      return;
    }

    if (key == 'A') {
      playConfirmSound();

      switch (menuIndex) {
        case 0:
          performFeeding(settings.openTimeMs, false);
          break;

        case 1:
          startEditMode(EDIT_OPEN_TIME);
          break;

        case 2:
          settings.autoModeEnabled = !settings.autoModeEnabled;

          if (settings.autoModeEnabled) {
            scheduleNextAutomaticFeed();
          }

          drawMenuScreen();
          break;

        case 3:
          startEditMode(EDIT_AUTO_INTERVAL);
          break;

        case 4:
          settings.soundEnabled = !settings.soundEnabled;
          drawMenuScreen();
          break;
      }

      return;
    }

    return;
  }


  // =====================================================
  // STATUS SCREEN CONTROLS
  // =====================================================
  if (currentScreen == SCREEN_STATUS) {
    if (key == 'D') {
      playBackSound();
      drawMenuScreen();
      return;
    }

    if (key == 'A') {
      playConfirmSound();
      drawStartScreen();
      return;
    }

    return;
  }


  // =====================================================
  // EDIT SCREEN CONTROLS
  // =====================================================
  if (currentScreen == SCREEN_EDIT) {
    if (key >= '0' && key <= '9') {
      if (editFresh) {
        editBuffer = "";
        editFresh = false;
      }

      if (editBuffer.length() < 6) {
        editBuffer += key;
        refreshEditScreen();
      }

      return;
    }

    if (key == '#') {
      editBuffer = "";
      editFresh = false;
      refreshEditScreen();
      return;
    }

    if (key == '*') {
      if (editBuffer.length() > 0) {
        editBuffer.remove(editBuffer.length() - 1);
      }

      editFresh = false;
      refreshEditScreen();
      return;
    }

    if (key == 'D') {
      playBackSound();
      drawMenuScreen();
      return;
    }

    if (key == 'A') {
      playConfirmSound();
      confirmEditValue();
      return;
    }

    return;
  }


  // =====================================================
  // ACTION SCREEN CONTROLS
  // =====================================================
  if (currentScreen == SCREEN_ACTION) {
    if (key == 'D') {
      playBackSound();
      drawStartScreen();
      return;
    }

    if (key == 'A') {
      playConfirmSound();
      performFeeding(settings.openTimeMs, false);
      return;
    }
  }
}

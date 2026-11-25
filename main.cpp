#include <Arduino.h>
#include "sounds.h"

// Konami code -> latch pulse (1 s)
// UP RIGHT DOWN DOWN DOWN -> calls easterEggSuccess()
// ESP32 polling version, with serial debug

const int UP_PIN    = 18;
const int DOWN_PIN  = 19;
const int LEFT_PIN  = 22;
const int RIGHT_PIN = 21;
const int A_PIN     = 32;
const int B_PIN     = 33;

const int OUT_PIN   = 25;  // MOSFET gate (latch)
const int BUZZER_PIN = 26;

const unsigned long SAMPLE_MS = 100;   // how often buttons are checked
const unsigned long LATCH_MS  = 1000;

enum Button { UP, DOWN, LEFT, RIGHT, BTN_B, BTN_A, BUTTON_COUNT };

const int buttonPins[BUTTON_COUNT] = {
  UP_PIN, DOWN_PIN, LEFT_PIN, RIGHT_PIN, B_PIN, A_PIN
};

const char* buttonNames[BUTTON_COUNT] = {
  "UP", "DOWN", "LEFT", "RIGHT", "B", "A"
};

// ---------- Sequences ----------

enum Action { ACTION_LATCH, ACTION_EASTER_EGG };

struct Sequence {
  const char*   name;
  const Button* steps;
  int           len;
  Action        action;
  int           progress;   // how many steps are currently matched
};

// up, up, down, down, left, right, left, right, b, a
const Button KONAMI[] = {
  UP, UP, DOWN, DOWN, LEFT, RIGHT, LEFT, RIGHT, BTN_B, BTN_A
};
const Button EGG_SEQ[] = {
  UP, RIGHT, DOWN, DOWN, DOWN
};

Sequence sequences[] = {
  { "Konami",   KONAMI,  sizeof(KONAMI)  / sizeof(KONAMI[0]),  ACTION_LATCH,      0 },
  { "EasterEgg", EGG_SEQ, sizeof(EGG_SEQ) / sizeof(EGG_SEQ[0]), ACTION_EASTER_EGG, 0 },
};
const int SEQ_COUNT = sizeof(sequences) / sizeof(sequences[0]);

// History must be at least as long as the longest sequence
const int HISTORY_LEN = 10;
static_assert(sizeof(KONAMI) / sizeof(KONAMI[0]) <= HISTORY_LEN, "HISTORY_LEN too small");
static_assert(sizeof(EGG_SEQ) / sizeof(EGG_SEQ[0]) <= HISTORY_LEN, "HISTORY_LEN too small");

int history[HISTORY_LEN];
int historyCount = 0;

// ---------- State ----------

unsigned long lastSample = 0;
uint8_t lastMask = 0;      // bitmask of buttons pressed in the previous sample

bool latchActive = false;
unsigned long latchStart = 0;

// ---------- History / matching ----------

void clearHistory() {
  historyCount = 0;
  for (int i = 0; i < SEQ_COUNT; i++) sequences[i].progress = 0;
}

void pushPress(int b) {
  if (historyCount < HISTORY_LEN) {
    history[historyCount++] = b;
  } else {
    for (int i = 1; i < HISTORY_LEN; i++) history[i - 1] = history[i];
    history[HISTORY_LEN - 1] = b;
  }
}

// Longest run of the most recent presses that matches the start of a sequence
int computeProgress(const Sequence& s) {
  int maxK = (historyCount < s.len) ? historyCount : s.len;
  for (int k = maxK; k > 0; k--) {
    bool ok = true;
    for (int i = 0; i < k; i++) {
      if (history[historyCount - k + i] != s.steps[i]) {
        ok = false;
        break;
      }
    }
    if (ok) return k;
  }
  return 0;
}

// ---------- Actions ----------

void startLatch() {
  konamiCodeSuccess(BUZZER_PIN);
  digitalWrite(OUT_PIN, HIGH);
  latchActive = true;
  latchStart = millis();
  Serial.printf("[%lu ms] *** KONAMI CODE ACCEPTED - latch HIGH ***\n", latchStart);
  delay(2000); //Żeby zamek miał czas się otworzyć
}

void startEasterEgg() {
  Serial.printf("[%lu ms] *** EASTER EGG CODE ACCEPTED - calling easterEggSuccess() ***\n",
                millis());
  easterEggSuccess(BUZZER_PIN);
}

// ---------- Printing helpers ----------

void printButtons(uint8_t mask, const char* sep) {
  if (mask == 0) {
    Serial.print("none");
    return;
  }
  bool first = true;
  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (mask & (1 << i)) {
      if (!first) Serial.print(sep);
      Serial.print(buttonNames[i]);
      first = false;
    }
  }
}

// ---------- Press handling ----------

void onButtonPressed(int b) {
  unsigned long now = millis();
  Serial.printf("[%lu ms] Pressed: %s\n", now, buttonNames[b]);

  if (latchActive) {
    Serial.println("  Ignored (latch is active)");
    return;
  }

  // Remember what each sequence expected BEFORE this press
  int oldProgress[SEQ_COUNT];
  uint8_t expectedMask = 0;
  for (int i = 0; i < SEQ_COUNT; i++) {
    oldProgress[i] = sequences[i].progress;
    expectedMask |= (1 << sequences[i].steps[oldProgress[i]]);
  }

  pushPress(b);

  // Compute new progress and check for a completed sequence
  int newProgress[SEQ_COUNT];
  int completed = -1;
  bool advanced = false;
  for (int i = 0; i < SEQ_COUNT; i++) {
    newProgress[i] = computeProgress(sequences[i]);
    if (newProgress[i] == sequences[i].len) completed = i;
    if (newProgress[i] == oldProgress[i] + 1) advanced = true;
  }

  if (completed >= 0) {
    Action action = sequences[completed].action;
    clearHistory();
    if (action == ACTION_LATCH) startLatch();
    else                        startEasterEgg();
    return;
  }

  for (int i = 0; i < SEQ_COUNT; i++) sequences[i].progress = newProgress[i];

  if (advanced) {
    Serial.print("  OK -");
    keySuccess(BUZZER_PIN);
    bool first = true;
    for (int i = 0; i < SEQ_COUNT; i++) {
      if (newProgress[i] > 0) {
        Serial.printf("%s %s %d/%d", first ? "" : ",",
                      sequences[i].name, newProgress[i], sequences[i].len);
        first = false;
      }
    }
    Serial.println();
  } else {
    Serial.print("  FAIL - expected ");
    keyFailure(BUZZER_PIN);
    printButtons(expectedMask, " or ");
    Serial.printf(", got %s\n", buttonNames[b]);

    bool any = false;
    for (int i = 0; i < SEQ_COUNT; i++) {
      if (newProgress[i] > 0) {
        Serial.printf("  Partial match kept - %s now at step %d/%d\n",
                      sequences[i].name, newProgress[i], sequences[i].len);
        any = true;
      }
    }
    if (!any) Serial.println("  Progress reset to 0");
  }
}

// ---------- Input ----------

uint8_t readMask() {
  uint8_t mask = 0;
  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (digitalRead(buttonPins[i]) == HIGH) mask |= (1 << i);
  }
  return mask;
}

void setup() {
  Serial.begin(115200);

  pinMode(OUT_PIN, OUTPUT);
  digitalWrite(OUT_PIN, LOW);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttonPins[i], INPUT_PULLDOWN);
  }

  Serial.println("Ready. Enter a code.");
}

void loop() {
  unsigned long now = millis();

  // Sample the buttons every SAMPLE_MS
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    uint8_t mask = readMask();

    if (mask != lastMask) {
      Serial.printf("[%lu ms] Inputs: ", now);
      printButtons(mask, "+");
      Serial.println();

      bool single = (mask != 0) && ((mask & (mask - 1)) == 0);  // exactly one bit set
      if (single) {
        for (int i = 0; i < BUTTON_COUNT; i++) {
          if (mask == (1 << i)) onButtonPressed(i);
        }
      } else if (mask != 0) {
        Serial.println("  Multiple buttons at once - not counted");
      }
    }

    lastMask = mask;
  }

  // End the latch pulse (non-blocking)
  if (latchActive && (now - latchStart) >= LATCH_MS) {
    digitalWrite(OUT_PIN, LOW);
    latchActive = false;
    clearHistory();
    Serial.printf("[%lu ms] Latch LOW\n", now);
  }
}
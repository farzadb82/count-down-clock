#include <MsTimer2.h>


#define HOURS_10    0
#define HOURS_1     1
#define MINUTES_10  2
#define MINUTES_1   3
#define SECONDS_10  4
#define SECONDS_1   5

uint8_t numbers[] {
  B00111111,     // 0
  B00000110,     // 1
  B01011011,     // 2
  B01001111,     // 3
  B01100110,     // 4
  B01101101,     // 5
  B01111101,     // 6
  B00000111,     // 7
  B01111111,     // 8
  B01101111,     // 9
};

uint8_t digits[] {
  A4,
  A5,
  2,
  3,
  4,
  5
};

volatile byte clock_time[] { 0, 0, 0, 0, 0, 0 };
volatile uint8_t DP_CONTROL = B00001010;
volatile uint8_t ACTIVE_DIGIT = digits[0];
void displayDigit(uint8_t digit, uint8_t number) {
  digitalWrite(ACTIVE_DIGIT, HIGH);
  ACTIVE_DIGIT = digits[digit];

  uint8_t pattern = numbers[number];
  for (uint8_t pin=6; pin<=13; pin++) {
    if (bitRead(pattern, pin-6) == 1) {
      digitalWrite(pin, LOW);
    } else {
      digitalWrite(pin, HIGH);
    }
    if (bitRead(DP_CONTROL, ACTIVE_DIGIT) == 1) {
      digitalWrite(13, LOW);
    }
  }

  digitalWrite(ACTIVE_DIGIT, LOW);
}

volatile uint8_t MAX_TIMER_CYCLES = sizeof(clock_time);
volatile uint8_t current_timer_cycle = 0;
void updateDisplay() {
  noInterrupts();

  uint8_t current_digit = current_timer_cycle;
  if (current_digit < sizeof(clock_time)) {
    displayDigit(current_digit, clock_time[current_digit]);
  } else {
    digitalWrite(ACTIVE_DIGIT, HIGH);
  }
  current_timer_cycle += 1;
  if (current_timer_cycle >= MAX_TIMER_CYCLES) {
    current_timer_cycle = 0;
  }

  interrupts();
}

void decrement_hours_10() {
  if (clock_time[HOURS_10] == 0) {
    clock_time[HOURS_10] = 2;
  } else {
    clock_time[HOURS_10] -= 1;
  }
}

void decrement_hours_1() {
  if (clock_time[HOURS_1] == 0) {
    decrement_hours_10();
    if (clock_time[HOURS_10] == 2) {
      clock_time[HOURS_1] = 3;
    } else {
      clock_time[HOURS_1] = 9;
    }
  } else {
    clock_time[HOURS_1] -= 1;
  }
}

void decrement_minutes_10() {
  if (clock_time[MINUTES_10] == 0) {
    decrement_hours_1();
    clock_time[MINUTES_10] = 5;
  } else {
    clock_time[MINUTES_10] -= 1;
  }
}

void decrement_minutes_1() {
  if (clock_time[MINUTES_1] == 0) {
    decrement_minutes_10();
    clock_time[MINUTES_1] = 9;
  } else {
    clock_time[MINUTES_1] -= 1;
  }
}

void decrement_seconds_10() {
  if (clock_time[SECONDS_10] == 0) {
    decrement_minutes_1();
    clock_time[SECONDS_10] = 5;
  } else {
    clock_time[SECONDS_10] -= 1;
  }
}

void decrement_seconds_1() {
  if (clock_time[SECONDS_1] == 0) {
    decrement_seconds_10();
    clock_time[SECONDS_1] = 9;
  } else {
    clock_time[SECONDS_1] -= 1;
  }
}


void setup() {
  for (uint8_t digit=0; digit<sizeof(digits); digit++) {
    pinMode(digits[digit], OUTPUT);
    digitalWrite(digits[digit], HIGH);
  }

  for (uint8_t pin=6; pin<=13; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  Serial.begin(9600);

  MsTimer2::set(1, updateDisplay);
  MsTimer2::start();
}

#define POT_PIN 0
#define LDR_PIN 1
int pot_val = 0;
int ldr_val = 0;
float diff;
void loop() {
  pot_val = analogRead(POT_PIN);
  //Serial.println(val);
  if (pot_val > 1000) {
    pot_val = 1000;
  } else {
    pot_val = pot_val / 4;
  }
  delay(pot_val);
  decrement_seconds_1();

  ldr_val = analogRead(LDR_PIN);
  if (ldr_val < 100) {
    // MIN brightness
    MAX_TIMER_CYCLES = sizeof(clock_time) + 18;
  } else {
    // MAX brightness
    MAX_TIMER_CYCLES = sizeof(clock_time);
  }
  Serial.print(ldr_val);
}

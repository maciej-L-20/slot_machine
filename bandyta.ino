#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <LiquidCrystal_I2C.h>

#define BUTTON 2
#define LEFT 9
#define MID 10
#define RIGHT 11
#define NUMPIXELS 8
#define DELAYVAL 500

String instruction = "Kliknij aby wylosowac!    ";
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_NeoPixel leds[3]
    = { Adafruit_NeoPixel(NUMPIXELS, LEFT, NEO_GRB + NEO_KHZ800),
        Adafruit_NeoPixel(NUMPIXELS, MID, NEO_GRB + NEO_KHZ800),
        Adafruit_NeoPixel(NUMPIXELS, RIGHT, NEO_GRB + NEO_KHZ800) };
int brightness = 25;
int allColors[5][3] = { { brightness, 0, 0 }, { brightness, brightness, 0 },
  { 0, brightness, 0 }, { 0, 0, brightness },
  { brightness, 0, brightness / 2 } };
int stages[4][3]
    = { { 100, 100, 100 }, { -1, 200, 200 }, { -1, -1, 300 }, { -1, -1, -1 } };
int colors[3][5] = {};
unsigned long last = 0;
long times[3] = { 0, 0, 0 };
long timer = 0;
long maxTimes[3] = { 100, 100, 100 };
int ledCounts[2] = { 0, 0 };
int ledCountsMax[2] = { 0, 0 };
bool prevPressed = false;
int stage = 0;
int score = 150, prevScore = 150, nextScore = 150;
long scoreTimer = 0;
long instructionTimer = 0;

void setup()
{
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(BUTTON, INPUT);
  for (int i = 0; i < 3; i++) {
    leds[i].begin();
    for (int j = 0; j < 3; j++)
      colors[i][j] = rand() % 5;
  }
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Bandyta!");
}

void loop()
{
  bool pressed = digitalRead(BUTTON) == HIGH;
  if (pressed && !prevPressed) {
    if (stage == 0 && score >= 0) {
      stage = 1;
      applyStage();
      for (int i = 0; i < 2; i++) {
        ledCounts[i] = 0;
        ledCountsMax[i] = rand() % 5 + 3;
      }
      lcd.setCursor(0, 0);
      lcd.print("  Losowanie...  ");
      changeScore(-20);
    } else if (stage == 3) {
      stage = 0;
      applyStage();
      for (int i = 0; i < 3; i++)
        times[i] = 0;
      timer = 0;
    }
  }
  unsigned long current = millis();
  long elapsed = current - last;
  if (stage > 0 && stage < 3) {
    if (ledCounts[stage - 1] == ledCountsMax[stage - 1]) {
      stage++;
      applyStage();
    }
    if (stage == 3) {
      if (colors[0][2] == colors[1][2] && colors[0][2] == colors[2][2]) {
        lcd.setCursor(0, 0);
        lcd.print(" Rozbiles Bank! ");
        changeScore(150);
      } else if (colors[0][2] == colors[1][2] || colors[0][2] == colors[2][2]
          || colors[1][2] == colors[2][2]) {
        lcd.setCursor(0, 0);
        lcd.print("  Mala Wygrana  ");
        changeScore(30);
      } else {
        lcd.setCursor(0, 0);
        lcd.print("  Przegrana :(  ");
      }
    }
  }
  for (int i = 0; i < 3; i++) {
    times[i] += elapsed;
    if (maxTimes[i] > -1 && times[i] > maxTimes[i]) {
      if (stage > 0 && i == stage)
        ledCounts[i - 1]++;
      times[i] -= maxTimes[i];
      for (int j = 0; j < 4; j++)
        colors[i][j] = colors[i][j + 1];
      colors[i][4] = rand() % 5;
    }
  }
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 5; j++) {
      int index = colors[i][j];
      leds[i].setPixelColor(j + 3,
          leds[i].Color(
              allColors[index][0], allColors[index][1], allColors[index][2]));
    }
    leds[i].show();
  }
  if (stage == 0) {
    instructionTimer += elapsed;
    Serial.println(instructionTimer);
    int offset = instructionTimer / 400 % instruction.length();
    lcd.setCursor(0, 0);
    lcd.print(instruction.substring(offset, instruction.length()));
    if (instruction.length() - offset < 16)
      lcd.print(instruction.substring(0, offset));
    }
  if (scoreTimer < 1000) {
    scoreTimer += elapsed;
    if (scoreTimer >= 1000)
      score = nextScore;
    else {
      score = prevScore
          + (int)(((float)scoreTimer / 1000) * (nextScore - prevScore));
    }
    lcd.setCursor(0, 1);
    lcd.print("Monety: ");
    lcd.print(score);
    lcd.print("   ");
  }
  last = current;
  prevPressed = pressed;
}

void applyStage()
{
  for (int i = 0; i < 3; i++)
    maxTimes[i] = stages[stage][i];
}

void changeScore(int difference)
{
  prevScore = score;
  nextScore = score + difference;
  if(nextScore < 0) {
    nextScore = 150;
    stage = 0;
    applyStage();
    lcd.setCursor(0, 0);
    lcd.print("    Nowa Gra    ");
  }
  scoreTimer = 0;
}
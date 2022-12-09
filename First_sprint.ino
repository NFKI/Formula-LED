/*
Mäng aadressiga LED-ribal "Formula LED"
   - Mängijate arv on piiratud nelja inimesega
   - Nupu hoidmine – kiirust juurde ( ainult prototüübil )
   - Nupu lahti laskmine - pidurdamine
   - Kiirendus kurvi tsoonis toob kaasa kiiruspiirangu kuni kurvi lõpuni
*/
#define LED_PIN 2     // LED pins
#define LED_NUM 100   // LED-ide arv
#define LED_BR 150    // Riba heledus
// LED-tihvtide arv vastavalt mängijate arvule
const byte pins[] = {3, 4};
#define MAX_SPEED 15  // max. kiirus
#define MIN_SPEED 4   // max. Kiirus kurvidel
#define WIN_SCORE 3  // võiduskoor
// =============================================
#include "FastLED.h"
CRGB leds[LED_NUM];
int turns[TURN_ZONES][2];
CRGB turnColor = CHSV(12, 255, 70);
const byte players = sizeof(pins);  // massiivi suurus == mängijate arv
// ============ SETUP =============
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.setBrightness(LED_BR);
  // Kõikide tihvtide initsialiseerimine
  for (int p = 0; p < players; p++) pinMode(pins[p], INPUT_PULLUP);
  newGame();    // alusta uus mäng
}
// ========== MUUTUJAD ==========
int pos[players];       // sprite-ide asukoht
int spd[players];       // sprite-ide kiirus
int score[players];     // skoor
bool drag[players];     // pidurdamise lipp
// ============= LOOP =============
void loop() {
  static uint32_t tmr, tmr2;
  // kiiruse õmberarvutus
  // taimer 200 ms
  if (millis() - tmr2 >= 200) {
    tmr2 = millis();
    for (int p = 0; p < players; p++) {         // kõikidele mängijatele
      if (!digitalRead(pins[p])) spd[p] += 3;   // kui nup on vajutatud - kiirendus
      else spd[p]--;                            // vastupidises olukorras - pidurdamine
      // rakkendame piirangud max. Kiirusele või kiirusele kurvidel
      spd[p] = constrain(spd[p], 0, drag[p] ? MIN_SPEED : MAX_SPEED);
    }
  }
  // sprite-ide liikumine
  // taimer 10 ms
  if (millis() - tmr >= 10) {
    tmr = millis();
    FastLED.clear();  // puhastame lindi
    // käime läbi kõik sprite-id
    for (int p = 0; p < players; p++) {
      pos[p] += spd[p];   // liigume praeguse kiirusega
      // kontrollime kas sprite jõudis finishisse
      if (pos[p] >= (LED_NUM * 10)) {   // riba lõppes, ring on läbitud
        pos[p] -= LED_NUM * 10;         // ring--
        score[p]++;                     // skoor +1
        // võidu skoor on saavutatud
        if (score[p] >= WIN_SCORE) {
          // riba saab värviks võitja värvi 
          for (int led = 0; led < LED_NUM; led += 3) {
            leds[led].setHue(p * 255 / players);
            FastLED.show();
            delay(10);
          }
          newGame();
          return;   // uus tsükkel
        }
      }
      // kaotajate positsioon, mängu lõpus, salvestab kaotajate värve
      leds[pos[p] / 10].setHue(p * 255 / players);
    }   // end
    FastLED.show();
  } // end of taimer
}
void newGame() {
  // skoor = NULL && kiirus = NULL
  for (int np = 0; np < players; np++) {
    pos[np] = spd[np] = score[np] = 0;
    drag[np] = 0;
  }
}

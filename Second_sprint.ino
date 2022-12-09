#define LED_PIN 2      // LED1 pin
#define LED_PIN2 3     // LED2 pin
#define LED_NUM 138    // LED-ide arv ( esimesel ribal )
#define LED_NUM2 162   // LED-ide arv ( teisel ribal )
#define LED_BR 150     // riba heledus

// Nuppude pinnide arv vastab mängijate arvule
const byte pins[] = {4, 5};

#define MAX_SPEED 15  // max. kiirus
#define MIN_SPEED 4   // max. Kiirus kurvidel
#define TURN_ZONES 2  // kurvide arv
#define TURN_MIN 10   // min. kurvide pikkus
#define TURN_MAX 20   // max. kurvide pikkus
#define BOOST_ZONES 2 //
#define BOOST_SIZE 1  //
#define WIN_SCORE 2   // võidu skoor

// =============================================
#include "FastLED.h"
CRGB leds[LED_NUM];
CRGB leds2[LED_NUM2];
int turns[TURN_ZONES][2];
int boosts[BOOST_ZONES];
CRGB turnColor = CHSV(12, 255, 70);
CRGB boostColor = CHSV(100, 10, 100);
const byte players = sizeof(pins);  // massiivi suurus == mängijate arv

// ============ SETUP =============
void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds2, LED_NUM2);
  FastLED.setBrightness(LED_BR);

  // nuppude initsialiseerimine
  for (int p = 0; p < players; p++) pinMode(pins[p], INPUT_PULLUP);

  randomSeed(analogRead(0));              // juhuslikkus veelgi juhuslikum
  int turnSize = (LED_NUM+LED_NUM2) / TURN_ZONES;    // plokki pikkus kui  LED-de arv/kuhvrite arv
  for (int t = 0; t < TURN_ZONES; t++) {  // kurvide arvu saamiseks
    // otsime kurvide algust iga plokki alguses
    turns[t][0] = random(t * turnSize, (t + 1) * turnSize - TURN_MAX);

    // otsime kurvi , lisades pikkus algusele
    turns[t][1] = turns[t][0] + random(TURN_MIN, TURN_MAX);
  }
  
  int boostSize = (LED_NUM+LED_NUM2) / BOOST_ZONES;    // plokkide pikkus kui LED-ide arv/kurvide arv
  for (int b = 0; b < BOOST_ZONES; b++) {  // kurvide arvu saamiseks
    // otsime kurvi algust iga plokki alguses
    boosts[b] = random(b * boostSize, (b + 1) * boostSize - BOOST_SIZE);
  }
  newGame();    // alustada uus mäng
}

// ========== ПЕРЕМЕННЫЕ ==========
int pos[players];       // sprite-ide asukoht
int spd[players];       // sprite-ide kiirus
int score[players];     // skoor
bool drag[players];     // pidurdamise lipp
bool accel[players];
int buttonPrev[players] = {0,0}; // TODO!!!!
// ============= LOOP =============
void loop() {
  static uint32_t tmr, tmr2;
  // kiiruse ümberarvutus
  // taimer 200 ms
  if (millis() - tmr2 >= 200) {
    tmr2 = millis();
    for (int p = 0; p < players; p++) {         // iga sprite-i jaoks  
      if (buttonPrev[p]==1 &&  !digitalRead(pins[p])) spd[p] += 3;   // kui nupp on vajutatud - kiirendus
      else spd[p]--;                            // vastupidises olukorras - pidurdamine
      buttonPrev[p] = digitalRead(pins[p]);
      Serial.println(digitalRead(pins[p]));
      // seadistame piirangud max kiirusele või kiirusele kurvidel
      spd[p] = constrain(spd[p], 0, drag[p] ? MIN_SPEED : MAX_SPEED);
    }
  }

  // sprite-ide liikumine
  // taimer 10 ms
  if (millis() - tmr >= 10) {
    tmr = millis();
    FastLED.clear();  // riba puhastamine

    // kurvide initsialiseerimine
    for (int t = 0; t < TURN_ZONES; t++) {              // iga kurvi jaoks
      for (int s = turns[t][0]; s < turns[t][1]; s++) { // stardilt finishini
        if(s<LED_NUM){
          leds[s] = turnColor;    // värime piksleid
        }
        else{
          leds2[LED_NUM+LED_NUM2-s] = turnColor;
        }
      }
    }

    for (int b = 0; b < BOOST_ZONES; b++) {              // iga kurvi jaoks
      for (int s = boosts[b]; s < (boosts[b] + BOOST_SIZE); s++) { // stardilt finishini
        if(s<LED_NUM){
          leds[s] = boostColor;    // värvime piksli
        }
        else{
          leds2[LED_NUM+LED_NUM2-s] = boostColor;
        }
      }
    }

    // käime läbi kõik spite-id
    for (int p = 0; p < players; p++) {
      pos[p] += spd[p];   // liigume praeguse pikkusega
      
        if (!digitalRead(pins[p])) {  // kui nupp on vajutatud
          for (int t = 0; t < TURN_ZONES; t++) {    // kontrollimekurvid
            // sprite asub kurvis
            if (pos[p] / 10 >= turns[t][0] && pos[p] / 10 < turns[t][1]) {
              drag[p] = 1;            // lipp kiiruse piiramisele
              spd[p] = MIN_SPEED;     // piirandame kiirust
            } else drag[p] = 0;       // nupp pole vajutatud – võtame piirangud maha
          }
        } else drag[p] = 0;       // nupp pole vajutatud – võtame piirangud maha
        
        if (!digitalRead(pins[p])) {  // nupp on vajutatud
          for (int b = 0; b < BOOST_ZONES; b++) {    // kontrollime kurvid
            // sprite on kurvis
            if (pos[p] / 10 >= boosts[b] && pos[p] / 10 < (turns[b] + BOOST_SIZE)) {
              accel[p] = 1;            // lipp kiiruse piiramisele
              spd[p] += 5;     // piirandame kiirust
            } else accel[p] = 0;       // nupp pole vajutatud – võtame maha piirangud
          }
        } else accel[p] = 0;       // nupp pole vajutatud – võtame maha piirangud
              

      // finishi kontroll
      if (pos[p] >= ((LED_NUM+LED_NUM2) * 10)) {   // riba lõpp, ring on läbitud
        pos[p] -= (LED_NUM+LED_NUM2) * 10;         // ring--
        score[p]++;                     // skoor +1

        // võidu skoor on saavutatud 
        if (score[p] >= WIN_SCORE) {

          // riba saab värviks võitja värvi
          for (int led = 0; led < (LED_NUM+LED_NUM2); led++) {
            if(led%3==0){
              if(led<LED_NUM){
                leds[led].setHue(p * 255 / players);
              }
              else{
                leds2[LED_NUM2+LED_NUM-led].setHue(p * 255 / players);
              }
            }
          FastLED.show();
          delay(10);
          }
          newGame();
          return;   // uus tsükkel
        }
      }

      // kui pole võitja – kaotaja osaukoht saab värviks vastava kaotaja värvi
      if(pos[p]<(LED_NUM*10)){
        leds[pos[p] / 10].setHue(p * 255 / players);
      }
      else{
        leds2[((LED_NUM2+LED_NUM)*10 - pos[p]) / 10].setHue(p * 255 / players);
      }
    }   // end for players
    FastLED.show();
  } // end of timer
}

void newGame() {
  // skoor = NULL && kiirus = NULL
  for (int np = 0; np < players; np++) {
    pos[np] = spd[np] = score[np] = 0;
    drag[np] = 0;
  }
}

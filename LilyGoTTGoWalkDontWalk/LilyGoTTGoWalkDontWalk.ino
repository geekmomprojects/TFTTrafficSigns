#include <Arduino.h>
#include <Ticker.h>
#include "image_padded.h"
#define TFT_eSPI_DEBUG
/*
  Sketch to show creation of a sprite with a transparent
  background, then plot it on the TFT.

  Example for library:
  https://github.com/Bodmer/TFT_eSPI

  A Sprite is notionally an invisible graphics screen that is
  kept in the processors RAM. Graphics can be drawn into the
  Sprite just as it can be drawn directly to the screen. Once
  the Sprite is completed it can be plotted onto the screen in
  any position. If there is sufficient RAM then the Sprite can
  be the same size as the screen and used as a frame buffer.

  A 16 bit Sprite occupies (2 * width * height) bytes in RAM.

  On a ESP8266 Sprite sizes up to 126 x 160 can be accomodated,
  this size requires 40kBytes of RAM for a 16 bit colour depth.
  
  When 8 bit colour depth sprites are created they occupy
  (width * height) bytes in RAM, so larger sprites can be
  created, or the RAM required is halved.
*/

#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)
#include <Button2.h>

#define BTN_LEFT  0
#define BTN_RIGHT 35

Button2 btnLeft = Button2(BTN_LEFT);
Button2 btnRight = Button2(BTN_RIGHT);
boolean bIsAnimating = false;

TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"


TFT_eSprite topSprite = TFT_eSprite(&tft);
TFT_eSprite bottomSprite = TFT_eSprite(&tft);

TFT_eSprite walkSprite = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
                                      // the pointer is used by pushSprite() to push it onto the TFT
TFT_eSprite handSprite = TFT_eSprite(&tft);


#define TRANSPARENT_COLOR 0
uint16_t fillSingleColorSprite(TFT_eSprite *sprite, const uint8_t *im, uint16_t w, uint16_t h, uint16_t color, uint8_t transparent = TRANSPARENT_COLOR) {
  sprite->createSprite(w,h);
  sprite->fillSprite(transparent);
  
  int index;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
       index = i*w + j;
       if (im[index] > 0) {
        sprite->drawPixel(j,i,color);
       }
    }
  }
}



int gWalkX = 20;
const int RECT_MARGIN = 2;
const int BITMAP_OFFSET = 3;
int angle = 0;

void drawSign() {
  int margin = RECT_MARGIN;
  topSprite.fillSprite(TFT_BLACK);
  //handSprite.pushToSprite(&topSprite, margin + BITMAP_OFFSET + 20, margin + BITMAP_OFFSET);
  handSprite.pushRotated(&topSprite, angle, TFT_BLACK);
  topSprite.drawRoundRect(margin, margin, tft.width() - 2*margin, tft.height()/2 - margin, 3, TFT_WHITE);

  bottomSprite.fillSprite(TFT_BLACK);
  walkSprite.pushToSprite(&bottomSprite, BITMAP_OFFSET + margin + gWalkX, margin + BITMAP_OFFSET);
  bottomSprite.drawRoundRect(margin, margin, tft.width() - 2*margin, tft.height()/2 - margin, 3, TFT_WHITE);

  topSprite.pushSprite(0, 0);
  bottomSprite.pushSprite(0, tft.height()/2);
}

int sign = 1;
#define MAX_WAVE_ANGLE 15
void doWave() {
  angle = angle + sign;
  if (abs(angle) > MAX_WAVE_ANGLE and sign > 0) {
    sign = -1; 
  } else if (abs(angle) > MAX_WAVE_ANGLE and sign < 0) {
    sign = 1;
  }
}

void doWalk() {
  gWalkX += 1;
  if (gWalkX >= tft.width()) {
    gWalkX = -WALK_WIDTH;
  }
  yield();
}


Ticker walkTicker;
Ticker waveTicker;
Ticker drawTicker;
boolean bIsWalking = false;
boolean bIsWaving = false;

void setup(void) {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  delay(100);

  topSprite.setColorDepth(8);
  topSprite.createSprite(tft.width(),tft.height()/2);
  topSprite.fillSprite(TFT_BLACK);

  bottomSprite.setColorDepth(8);
  bottomSprite.createSprite(tft.width(),tft.height()/2);
  bottomSprite.fillSprite(TFT_BLACK);

  walkSprite.setColorDepth(8);
  fillSingleColorSprite(&walkSprite, walkBitmap, WALK_WIDTH, WALK_HEIGHT, TFT_GREEN);
  handSprite.setColorDepth(8);
  fillSingleColorSprite(&handSprite, handBitmap, HAND_WIDTH, HAND_HEIGHT, TFT_RED);

  
  btnLeft.setClickHandler(click);
  btnLeft.setLongClickHandler(longClick);
  btnRight.setClickHandler(click);
  btnRight.setLongClickHandler(longClick);

  drawSign();
  // Set regular screen updates
  drawTicker.attach(.043, drawSign);
}

void toggleWalking() {
  if (bIsWalking) {
    walkTicker.detach();
    bIsWalking = false;
  } else {
    walkTicker.attach(0.033, doWalk);
    bIsWalking = true;
  }
}

void toggleWaving() {
  if (bIsWaving) {
    waveTicker.detach();
    bIsWaving = false;
  } else {
    waveTicker.attach(0.0233, doWave);
    bIsWaving = true;
  }
}

void longClick(Button2& btn) {
  if (btn == btnLeft) {
    toggleWalking();
  } else if (btn == btnRight) {
    toggleWaving();
  }
}

void click(Button2& btn){
  if (btn == btnLeft) {
    toggleWalking();
  } else if (btn == btnRight) {
    toggleWaving();
  }
}

void loop() {
  btnLeft.loop();
  btnRight.loop();
}

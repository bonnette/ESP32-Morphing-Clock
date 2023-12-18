/*--------------------------------------------------------------------
/ Larry Bonnette December 2023
/
/ Morphing Clock with PxMatrix library using My PCB and 
/ WEMOS D1 MINI ESP32. My PCB is uses wiring from PxMatrix library page
/ https://github.com/2dom/PxMatrix 
/ Time sync uses the ezTime library, instead of NTP
/ https://github.com/ropg/ezTime
/ and the morphing library by HariFun
/ https://www.instructables.com/Morphing-Digital-Clock/
/--------------------------------------------------------------------*/

// From original Hari Wiguna repository
// Morphing Clock by Hari Wiguna, July 2018
//
// Thanks to:
// Dominic Buchstaller for PxMatrix
// Tzapu for WifiManager
// Stephen Denne aka Datacute for DoubleResetDetector
// Brian Lough aka WitnessMeNow for tutorials on the matrix and WifiManager


// This is how many color levels the display shows - the more the slower the update
//#define PxMATRIX_COLOR_DEPTH 4

// Defines the speed of the SPI bus (reducing this may help if you experience noisy images)
//#define PxMATRIX_SPI_FREQUENCY 20000000

// Creates a second buffer for backround drawing (doubles the required RAM)
//#define PxMATRIX_double_buffer true

#include <SPIFFS.h>

#include "params.h"
#include "Digit.h"
#include "Digitsec.h"
#include "TinyFont.h"
#include <PxMatrix.h>
#include <WiFi.h>
#include <ezTime.h>

// Pins for LED MATRIX

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// needed EZtime variables

int hh;
int mm;
int ss;

String ehh;
String emm;
String ess;

int prevhh;
int prevmm;
int prevss;

// Time Colors
int clrRed = 0;
int clrBlu = 100;
int clrGrn = 0;

#define matrix_width 64
#define matrix_height 32

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 30;  //30-70 is usually fine

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);
//PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);



void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}

//=== SEGMENTS ===

Digit digit0(&display, 0, 63 - 1 - 9 * 1, 8, display.color565(clrRed, clrGrn, clrBlu));
Digit digit1(&display, 0, 63 - 1 - 9 * 2, 8, display.color565(clrRed, clrGrn, clrBlu));
Digit digit2(&display, 0, 63 - 4 - 9 * 3, 8, display.color565(clrRed, clrGrn, clrBlu));
Digit digit3(&display, 0, 63 - 4 - 9 * 4, 8, display.color565(clrRed, clrGrn, clrBlu));
Digit digit4(&display, 0, 63 - 7 - 9 * 5, 8, display.color565(clrRed, clrGrn, clrBlu));
Digit digit5(&display, 0, 63 - 7 - 9 * 6, 8, display.color565(clrRed, clrGrn, clrBlu));

void display_update_enable(bool is_enable) {
  if (is_enable) {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 2000, true);
    timerAlarmEnable(timer);
  } else {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
}

int epoch = 1;
int prevEpoch = 0;

// <-------------------------------------- Begin Setup ----------------------------
void setup() {

  Serial.begin(115200);
  delay(500);
  Serial.println("");
  Serial.print("Connecting");

  //connect to wifi network
  WiFi.begin(wifi_ssid, wifi_pass);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());

  delay(500);

  // Wait for EZtime to establish connection
  waitForSync();


  // Define your display layout here, e.g. 1/8 step, and optional SPI pins begin(row_pattern, CLK, MOSI, MISO, SS)
  display.begin(16);
  //display.begin(8, 14, 13, 12, 4);
  delay(500);

  // TinyFont usage (String)
  //TFDrawText(&display, String("  STARTING UP  "), 0, 13, display.color565(100, 100, 100));
  //display.display(display_draw_time);


  //delay(3000);
  //display_update_enable(false);
  //display.clearDisplay();
  display_update_enable(true);



  // Define multiplex implemention here {BINARY, STRAIGHT} (default is BINARY)
  //display.setMuxPattern(BINARY);
  // Set the multiplex pattern {LINE, ZIGZAG,ZZAGG, ZAGGIZ, WZAGZIG, VZAG, ZAGZIG} (default is LINE)
  //display.setScanPattern(LINE);

  // Rotate display
  //display.setRotate(true);

  // Flip display
  //display.setFlip(true);

  // Control the minimum color values that result in an active pixel
  //display.setColorOffset(5, 5,5);

  // Set the multiplex implemention {BINARY, STRAIGHT} (default is BINARY)
  //display.setMuxPattern(BINARY);

  // Set the color order {RRGGBB, RRBBGG, GGRRBB, GGBBRR, BBRRGG, BBGGRR} (default is RRGGBB)
  //display.setColorOrder(RRGGBB);

  // Set the time in microseconds that we pause after selecting each mux channel
  // (May help if some rows are missing / the mux chip is too slow)
  //display.setMuxDelay(0,1,0,0,0);

  // Set the number of panels that make up the display area width (default is 1)
  //display.setPanelsWidth(2);

  // Set the brightness of the panels (default is 255)
  //display.setBrightness(50);

  // Set driver chip type
  //display.setDriverChip(FM6124);

  // Draw Colons
  display.fillScreen(display.color565(0, 0, 0));
  digit1.DrawColon(display.color565(clrRed, clrGrn, clrBlu));
  digit3.DrawColon(display.color565(clrRed, clrGrn, clrBlu));

}  // <------------------------------ End Setup -------------------------------------

//  <----------------------------------- Begin Loop -------------------------------------

void loop() {

  Timezone myTZ;
  myTZ.setLocation("America/Chicago");

  //display.print(myTZ.dateTime("H:i:s"));
  //display.display(display_draw_time);
  //display_update_enable(true);
  //delay(3000);
  //display_update_enable(false);

  String TimeHolder = myTZ.dateTime("H:i:s");
  //int colonPosition = TimeHolder.indexOf(':');
  ehh = TimeHolder.charAt(0);
  ehh += TimeHolder.charAt(1);
  hh = ehh.toInt();
  emm = TimeHolder.charAt(3);
  emm += TimeHolder.charAt(4);
  mm = emm.toInt();
  ess = TimeHolder.charAt(6);
  ess += TimeHolder.charAt(7);
  ss = ess.toInt();

  /*Serial.println(TimeHolder);
  Serial.println(hh + ":" + mm + ":" + ss);
  display.setCursor(0, 20);
  display.print(hh + ":" + mm + ":" + ss);
  display_update_enable(true);*/

  if (epoch != prevEpoch) {

    if (prevEpoch == 0) {  // If we didn't have a previous time. Just draw it without morphing.
      Serial.println("Reached first time display");
      digit0.Draw(ss % 10);
      digit1.Draw(ss / 10);
      digit2.Draw(mm % 10);
      digit3.Draw(mm / 10);
      digit4.Draw(hh % 10);
      digit5.Draw(hh / 10);
    }

    else {
      // epoch changes every miliseconds, we only want to draw when digits actually change.
      if (ss != prevss) {
        int s0 = ss % 10;
        int s1 = ss / 10;
        if (s0 != digit0.Value()) digit0.Morph(s0);
        if (s1 != digit1.Value()) digit1.Morph(s1);
        prevss = ss;
      }

      if (mm != prevmm) {
        int m0 = mm % 10;
        int m1 = mm / 10;
        if (m0 != digit2.Value()) digit2.Morph(m0);
        if (m1 != digit3.Value()) digit3.Morph(m1);
        prevmm = mm;
      }

      if (hh != prevhh) {
        int h0 = hh % 10;
        int h1 = hh / 10;
        if (h0 != digit4.Value()) digit4.Morph(h0);
        if (h1 != digit5.Value()) digit5.Morph(h1);
        prevhh = hh;
      }
    }
  }


  epoch++;
  //Serial.println(epoch);
  if (epoch >= 1000) {
    epoch = 2;
    prevEpoch = 1;
    //Serial.println("Adjusted epoch");
  }
  prevEpoch = epoch + 1;



}  // <-------------------------- End Loop ----------------------------------------
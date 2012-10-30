#include "SPI.h"
#include "WS2801.h"

/**
 * Snow pattern for Vermillion's "Winter Lights" show 2012.
 */

// CONSTANTS...

// Visual configs.
static unsigned int wait = 0;
static unsigned int chance_of_snow = 8;
uint32_t snow_color = Color(40, 60, 70);

// System configs.
int pins[] = {
  2, // data  #1
  3, // clock #1
  4, // data  #2
  5  // clock #2
};
const unsigned int num_cols = 4;
const unsigned int num_rows = 20;
const unsigned int num_pins_per_strip = 40;

// Light data holders.
WS2801 strip1 = WS2801(num_pins_per_strip, pins[0], pins[1]);
WS2801 strip2 = WS2801(num_pins_per_strip, pins[2], pins[3]);

// Snow holder (4X20).
boolean snow_matrix[num_cols][num_rows] = {0};
//memset(snow_matrix, false, sizeof(snow_matrix));

// REQUIRED...

void setup() {
  strip1.begin();
  strip2.begin();
  randomSeed(analogRead(0));
  
  tester_flakes();
  
  // Debug.
  Serial.begin(115200);
  Serial.println("Ready to send data."); 
}

void loop() {  
  make_it_snow();
  delay(wait);
}


// WORK HORSES...

// Creation debug.
void tester_flakes() {
  snow_matrix[1][15] = true;
  snow_matrix[0][8]  = true;
  snow_matrix[1][17] = true;
  snow_matrix[0][3]  = true;
  //snow_matrix[2][5] = true;
  //snow_matrix[3][19] = true;
  //snow_matrix[2][10] = true;
  //snow_matrix[3][4] = true; 
}

// Master function.
void make_it_snow() {
  make_flakes();
  // Loop through snow.
  int r, c;
  for (c=0;c<num_cols;c++) {
    for (r=0;r<num_rows;r++) {
      if (snow_matrix[c][r] == true) {
        draw_flake(c, r, true);
        move_flake(c, r);
      }
      else {
        // @todo Could be more efficient.
        // Don't need to wipe if was already dark.
        draw_flake(c, r, false);
      }
    } 
  }
  strip1.show();
  strip2.show();
  /*
  x = map(x, 1, 4, 0, 80);
  */
}

// Generate new snow.
void make_flakes() {
  int i;
  // Loop through columns.
  for(i=0;i<num_cols;i++) {
    // Should we add a new flake?
    if( random(0, chance_of_snow) == 1 ) {
      snow_matrix[i][num_rows-1] = true;
    }
  }
}

// Shift known flakes to new position.
void move_flake(int c, int r) {
  // Out with the old.
  snow_matrix[c][r] = false;
  // In with the new.
  if (r > 0) {
    snow_matrix[c][r-1] = true;
  }
}

// Translate flake positions into data output.
void draw_flake(int c, int r, boolean snow) {
  // Calculate real pixel.
  int strip_col = c % (num_pins_per_strip/num_rows);
  int pixel = (strip_col * num_rows) + r;
  
  Serial.print("\t sc: ");
  Serial.print(strip_col);
  Serial.print("\t c,r: ");
  Serial.print(c);
  Serial.print(",");
  Serial.print(r);
  Serial.println();
  
  // Dealing with reversing second strip, should be linked to system vars.
  if (strip_col == 1) {
    pixel = map(pixel, 20, 39, 39, 20);
  }
  // Draw or delete.
  if (snow) {
    strip1.setPixelColor(pixel, snow_color);
  }
  else {
    strip1.setPixelColor(pixel, Color(0, 0, 0));
  }
  // Debug.
  /*
  Serial.print("c: ");
  Serial.print(c);
  Serial.print("\t r: ");
  Serial.print(r);
  Serial.print("\t p: ");
  Serial.print(pixel);
  Serial.println();
  */
}


/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}


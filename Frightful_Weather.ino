
#include "SPI.h"
#include "WS2801.h"
#include <MemoryFree.h>

/**
 * Patterns for Vermillion's "Ocillate" show. Seattle, 2012.
 */

/**
 * Weather Configs ...
 * 0 = Wait delay.
 * 1 = Min sparseness of active pixels.
 * 2 = Speed range of active pixels.
 * 3 = Flurie chance (laternal moves).
 */
const unsigned int seasons = 3;
static unsigned int configs[][4] = {
  // Snow.
  {100, 25, 4, 2},
  // Rain.
  {25, 2, 1, 0},
  // Leaves.
  {70, 26, 5, 5}
};
uint32_t colors[seasons][5] = {
  {
    // Snow.
    Color(40, 45, 60),
    Color(40, 45, 60),
    Color(40, 45, 60),
    Color(40, 45, 60),
    Color(40, 45, 60)
  }, {
    // Rain.
    Color(79, 104, 239),
    Color(21, 124, 197),
    Color(39, 154, 207),
    Color(79, 104, 239),
    Color(21, 124, 197)
  }, {
    // Leaves.
    Color(224, 95, 0),
    Color(220, 167, 9),
    Color(211, 46, 9),
    Color(228, 64, 1),
    Color(200, 44, 10)
  }
};
/*
// The old way.
static unsigned int wait = 50;
static unsigned int chance_of_snow_min = 26;
static unsigned int speed_range = 5;
unsigned int fluries = 4; // Intended for sensor.
*/

// CONSTANTS...
boolean debug_mode = false;

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
int snow_matrix[num_cols][num_rows] = {0};
//memset(snow_matrix, false, sizeof(snow_matrix));
unsigned int chance_of_snow = 2; // Starting value.
unsigned int speed_throttle = 1;
unsigned int cycles = 0;
unsigned int master_season = 0;
static unsigned long lWaitMillis;
static unsigned int master_wait = (20 * 1000);
uint32_t bg_color = Color(0, 0, 0);

// REQUIRED...

void setup() {
  strip1.begin();
  strip2.begin();
  randomSeed(analogRead(0));
  
  //tester_flakes();
  
  // Debug.
  if(debug_mode) {
    Serial.begin(115200);
    Serial.println("Ready to send data.");
  }
}

void loop() {
  // Use season configs.
  unsigned int wait = configs[master_season][0];
    
  // Rotate through seasons.
  if( (long)( millis() - lWaitMillis ) >= 0) {
    //  Rollover has occured...
    master_season++;
    lWaitMillis += master_wait;  // Check again one second later.
    // Draw even during a chance moment.
    make_it_snow();
  }
  else {  
    // Still before the next rollover time.
    make_it_snow();
  }
  // Keep within available seasons.
  if (master_season >= seasons) {
    master_season = 0;
  }

  if(debug_mode) {
    //Serial.print("freeMemory()=");
    //Serial.println(freeMemory());
    cycles++;
    String msg = "<< << << #";
    msg += cycles;
    //report(0, 0, 0, 0, msg);
  }

  report(5,5,5,5, "season: " + (String) master_season + " chance: " + (String) chance_of_snow);

  delay(wait);
}


// WORK HORSES...

// Creation debug.
void tester_flakes() {
  snow_matrix[1][15] = 1;
  snow_matrix[0][8]  = 1;
  snow_matrix[1][17] = 1;
  snow_matrix[0][3]  = 1;
  snow_matrix[2][5]  = 1;
  snow_matrix[3][19] = 1;
  snow_matrix[2][10] = 1;
  snow_matrix[3][4]  = 1; 
}

// Master function.
void make_it_snow() {
  // Use season configs.
  unsigned int speed_range = configs[master_season][2];
  
  make_flakes();
  // Loop through snow.
  int r, c;
  for (c=0;c<num_cols;c++) {
    for (r=0;r<num_rows;r++) {
      if (snow_matrix[c][r] > 0) {
        //report(c, r, snow_matrix[c][r], speed_throttle, "bothering");
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
  speed_throttle++;
  if (speed_throttle > speed_range) {
    speed_throttle = 1;
  }
  strip1.show();
  strip2.show();
}

// Generate new snow.
void make_flakes() {
  // Use season configs.
  unsigned int chance_of_snow_min = configs[master_season][1];
  unsigned int speed_range = configs[master_season][2];
  
  // Change the weather?
  switch (random(0,3)) {
    case 1:
      chance_of_snow += 1;
      break;
      
    case 2:
      chance_of_snow -= 1;
      break;
      
    case 3:
    default:
      break;
  }
  
  // Protect boundaries.
  if (chance_of_snow <= 1) {
   chance_of_snow = 2;
  }
  else if (chance_of_snow > chance_of_snow_min) {
    chance_of_snow = chance_of_snow_min;
  }

  /*
   // Seed the chance.
   chance_of_snow = random(1, chance_of_snow_min);
   // Enhance distribution.
   // (20 = 100, 12 = 4, 10 = 0, 5 = 25, 1  = 81)
   int mid_diff = abs(chance_of_snow - (chance_of_snow_min/2));
   chance_of_snow = pow(mid_diff, 2);
  */
  
  // Add a new flake?
  if( random(0, chance_of_snow) == 1) {
    
    // Pick a speed
    int s = random(1, speed_range);
    // Pick a column
    int c = random(0, num_cols);
     // Prevent neighbors.
     if (snow_matrix[c][num_rows-2] == 0) {
       snow_matrix[c][num_rows-1] = s;
     }
     
  }

}

// Shift known flakes to new position.
void move_flake(int c, int r) {
  // Use season configs.
  unsigned int fluries = configs[master_season][3];
  
  int s = snow_matrix[c][r];
  
  //report(c, r, s, speed_throttle, "regardless");
  
  if (s >= speed_throttle) {
  
    // Out with the old.
    snow_matrix[c][r] = 0;
    
    // Can't move it on the ground.
    if (r > 0) {
      // Lateral movement.
      if( random(0, fluries) == 1 ) {
        // Pick direction.
        int dir = random(0, 2);
        switch (dir) {
          case 1:
            // Move left, and wrap around.
            //report(c, r, s, speed_throttle, "left");
            if (c > 0) {
              snow_matrix[c-1][r-1] = s;
            }
            else {
              snow_matrix[num_cols-1][r-1] = s;
            }
            break;
          case 2:
            // Move right, with care.
            //report(c, r, s, speed_throttle, "right");
            if (c < num_cols-1) {
              snow_matrix[c+1][r-1] = s;
            }
            else {
              snow_matrix[0][r-1] = s;
            }
            break;
          default:
            // Just in case.
            //report(c, r, s, speed_throttle, "just in case");
            if (r > 0) {
              snow_matrix[c][r-1] = s;
            }
        }
      }
      // Move downward.
      else {
        //report(c, r, s, speed_throttle, "down");
        snow_matrix[c][r-1] = s;
      }
    } // End row careful.

  } // End speed.

}

// Translate flake positions into data output.
void draw_flake(int c, int r, boolean snow) {
  // Calculate real pixel.
  int strip_col = c % (num_pins_per_strip/num_rows);
  int pixel = (strip_col * num_rows) + r;
  
  /*
  Serial.print("\t sc: ");
  Serial.print(strip_col);
  Serial.print("\t c,r: ");
  Serial.print(c);
  Serial.print(",");
  Serial.print(r);
  Serial.println();
  */
  
  // Dealing with reversing second strip, should be linked to system vars.
  if (strip_col == 1) {
    pixel = map(pixel, 20, 39, 39, 20);
  }
  // Draw or delete.
  if (snow) {
    if (c < 2) {
      //strip1.setPixelColor(pixel, colors[snow_matrix[c][r]]);
      strip1.setPixelColor(pixel, colors[master_season][snow_matrix[c][r]]);
    }
    else {
      //strip2.setPixelColor(pixel, colors[snow_matrix[c][r]]);
      strip2.setPixelColor(pixel, colors[master_season][snow_matrix[c][r]]);
    }
  }
  else {
    if (c < 2) {
      strip1.setPixelColor(pixel, bg_color);
    }
    else {
      strip2.setPixelColor(pixel, bg_color);
    }
  }
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

void report (int c, int r, int s, int st, String msg) {

  if(debug_mode) {
    Serial.print("c: ");
    Serial.print(c);
    Serial.print("\t r: ");
    Serial.print(r);
    Serial.print("\t s: ");
    Serial.print(s);
    Serial.print("\t st: ");
    Serial.print(st);
    Serial.print("\t c: ");
    Serial.print(cycles);
  
    Serial.print("\t msg: ");
    Serial.print(msg);
  
    Serial.println();
  }

}


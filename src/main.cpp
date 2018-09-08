#include <Arduino.h>
#include <OctoWS2811.h>

#define TOTAL_STRIPS 50
#define NUM_STRIPS 8
#define NUM_LEDS 50
#define CONTROLED_LEDS 400
#define DELAY 17

// states
#define INIT 0
#define ACTIVE 1

// commands
#define NO_COMMAND -1
#define INIT_COMMAND 0

#define NO_PARAMETER -1
#define OFFSET_PARAMETER 0

#define CYCLE_COMMAND 1
#define CYCLE_PARAMETER 0

#define DRUM_COMMAND 2
#define DRUM_PARAMETER 0

#define CYMBOLS_COMMAND 3
#define CYMBOLS_PARAMETER 0

#define PUMP_COMMAND 4
#define PUMP_PARAMETER 0
#define PUMP_FREQ .4
#define PUMP_RATE .01
// 180/256
#define PUMP_CYCLE_RATE 0.703125

#define CYCLE_ADV 1
// 2*pi/256
#define CYCLE_FREQ 0.0245436926

#define DRUM_TIME 200
#define DRUM_SUSTAIN 150
#define DRUM_INIT_RADIUS 30.0
#define DRUM_RATE 0.13
#define DRUM_BORDER 10.0

#define CYMBOL_DECAY 5000
#define CYMBOL_SUSTAIN 500
#define CYMBOL_FREQ .0008
#define R_PHASE -.012
#define G_PHASE -.0128
#define B_PHASE -.0132

#define color(r,g,b) (((r)<<16) + ((g)<<8) + (b))
#define colorMap(val,r,g,b) color(max(0,map(val,0,1000,-(r)/2,(r))),max(0,map(val,0,1000,-(g)/2,(g))),max(0,map(val,0,1000,-(b)/2,(b))))
#define asRed(r) ((r)<<16)
#define asGreen(g) ((g)<<8)
#define asBlue(b) (b)
#define fromRed(r) (((r)>>16)&0xff)
#define fromGreen(g) (((g)>>8)&0xff)
#define fromBlue(b) ((b)&0xff)
#define mixColor(x,c1,c2) color(map(x,0,1000,fromRed(c1),fromRed(c2)), map(x,0,1000,fromGreen(c1),fromGreen(c2)), map(x,0,1000,fromBlue(c1),fromBlue(c2)))

// TODO:
// set delay

float distances[CONTROLED_LEDS];
float phases[CONTROLED_LEDS];
float drumDistances[CONTROLED_LEDS*5];

DMAMEM int displayMemory[NUM_LEDS*6];
int drawingMemory[NUM_LEDS*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(NUM_LEDS, displayMemory, drawingMemory, config);

int rainbowColors[180];

long lastRender = millis();
long lastDrumHit[] = {0,0,0,0,0};
int32_t drumColors[] = {0xfe00000, 0xf9fe000, 0x00fe030, 0x0003fe0, 0x9d00fe0};
int cycle = 0;

int cymbol = 0;
long lastCymbolHit = 0;

int pump = 0;

int state = INIT;

// commands
int command = NO_COMMAND;
int parameter = NO_PARAMETER;

// parameters
int offset = 0;

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue) {
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}

int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness) {
	unsigned int red, green, blue;
	unsigned int var1, var2;

	if (hue > 359) hue = hue % 360;
	if (saturation > 100) saturation = 100;
	if (lightness > 100) lightness = 100;

	// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
	if (saturation == 0) {
		red = green = blue = lightness * 255 / 100;
	} else {
		if (lightness < 50) {
			var2 = lightness * (100 + saturation);
		} else {
			var2 = ((lightness + saturation) * 100) - (saturation * lightness);
		}
		var1 = lightness * 200 - var2;
		red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
		green = h2rgb(var1, var2, hue) * 255 / 600000;
		blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
	}
	return (red << 16) | (green << 8) | blue;
}

void resetCommand() {
  parameter = NO_PARAMETER;
  command = NO_COMMAND;
}

void precompute() {
  for (int i=0; i< NUM_STRIPS; i++) {
    for (int j=0; j< NUM_LEDS; j++) {
      float dx = 3*(TOTAL_STRIPS*.5 - (i+offset));
      float dy = (NUM_LEDS*.5 - j);
      float distance = sqrt(dx*dx + dy*dy);
      distances[i*NUM_LEDS+j] = distance;
      phases[i*NUM_LEDS+j] = .002*pow(distance,2.0);
      for (int k=0; k < 5; k++) {
        dx = 3*(TOTAL_STRIPS*.16666*(k + 1.0) - (i+offset));
        drumDistances[i*NUM_LEDS+j + k*CONTROLED_LEDS] = sqrt(dx*dx + dy*dy);
      }
    }
  }
}

void readData() {
  if (Serial.available()) {
    int value = Serial.read();
    switch (command) {
      case NO_COMMAND:
        command = value;
        parameter = 0;
        break;
      case INIT_COMMAND:
        switch (parameter) {
          case OFFSET_PARAMETER:
            offset = value;
            precompute();
            state = ACTIVE;
          default:
            resetCommand(); break;
        }
        break;
      case CYCLE_COMMAND:
        switch (parameter) {
          case CYCLE_PARAMETER:
            cycle = value * CYCLE_ADV;
          default:
            resetCommand(); break;
        }
        break;
      case DRUM_COMMAND:
        switch (parameter) {
          case DRUM_PARAMETER:
            lastDrumHit[value] = millis();
          default:
            resetCommand(); break;
        }
        break;
      case PUMP_COMMAND:
        switch (parameter) {
          case PUMP_PARAMETER:
            pump = value;
          default:
            resetCommand(); break;
        }
        break;
      case CYMBOLS_COMMAND:
        switch (parameter) {
          case CYMBOLS_PARAMETER:
            cymbol = value;
            lastCymbolHit = millis();
          default:
            resetCommand(); break;
        }
        break;
      default:
        resetCommand(); break;
    }
  }
}

void setup() {
  for (int i=0; i<180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }

  leds.begin();
  Serial.begin(9600);
}

long attackDecay(long time, long attack, long decay) {
  if (time < attack)
    return map(time, 0, attack, 0, 1000);
  if (time < attack+decay) {
    int val = (int) (1000.0 * (time-attack) / decay);
    return map(val, 0, 1000, 1000, 0);
  }
  return 0;
}

void draw() {
  long cymbolTime = millis()-lastCymbolHit;
  int env = 0;
  int cymbolDecay = map(cymbol, 0, 255, 0, CYMBOL_DECAY);
  int cymbolSustain = map(cymbol, 0, 255, 0, CYMBOL_SUSTAIN);
  if (cymbolTime < cymbolDecay) {
    int val = (int) (1000.0 * cymbolTime / cymbolDecay);
    env = map(val, 0, 1000, 1000, 0);
  }
  float cfreq = CYMBOL_FREQ * attackDecay(cymbolTime, 0, 2*cymbolDecay) / 1000.0;

  for(uint8_t column=0; column<8; column++) {
    int x = column;
    int xoffset = x * NUM_LEDS;

    for(uint16_t i=0; i<NUM_LEDS; i++) {
      float dist = distances[xoffset + i];

      int pumpColor = rainbowColors[abs((int) (dist*PUMP_FREQ - (millis()-lastCymbolHit) * PUMP_RATE + cycle*PUMP_CYCLE_RATE))%180];
      int baseColor = mixColor(map(pump,0,255,0,1000), 0x808080, pumpColor);
      int pedalVal = (int) (500.0*(1.0+sin(CYCLE_FREQ*cycle + phases[xoffset + i])));
      uint32_t color = mixColor(pedalVal, 0x000000, baseColor);

      for(uint8_t drum=0; drum<5; drum++) {
        long drumTime = millis() - lastDrumHit[drum];

        if (drumTime <= DRUM_TIME) {
          float drumDist = drumDistances[xoffset + i + drum*CONTROLED_LEDS];
          float drumSize = drumTime * DRUM_RATE + DRUM_INIT_RADIUS;
          if (drumDist < drumSize) {
            int drumColor = drumDist > drumSize - DRUM_BORDER ? 0x000000 : drumColors[drum];
            int drumVal = drumTime < DRUM_SUSTAIN ? 1000 : map(drumTime, DRUM_SUSTAIN, DRUM_TIME, 1000, 0);
            color = mixColor(drumVal, color, drumColor);
          }
        }
      }

      // cymbol / stompbox
      // Atack decay curve base on hit time
      // 3 concentric sin waves, r, g and b
      // Same freq, phase is a function of time * c, where c varies slightly for each
      // for each wave, create a high colors from existing color, c: c|0xFF0000, c|0x00FF00, c|0x0000FF
      // also create low colors: c&0x00FFFF, c&0xFF00FF, c&0xFFFF00
      // if val=sin(freq*dist + cPhase*time) > 0, mixColor(val, c, highColor) otherwise mixColor(-val, c, lowColor)
      // combine colors something like this:
      if (env > 0) {
        // this is not actually mixing to zero
        int rval = (int) (env*cos((cfreq*dist + R_PHASE)*cymbolTime));
        int gval = (int) (env*cos((cfreq*dist + G_PHASE)*cymbolTime));
        int bval = (int) (env*cos((cfreq*dist + B_PHASE)*cymbolTime));
        uint32_t r = rval < 0 ? 0xff0000 : 0x000000;
        uint32_t g = gval < 0 ? 0x00ff00 : 0x000000;
        uint32_t b = bval < 0 ? 0x0000ff : 0x000000;
        uint32_t stompColor = r | g | b;
        color = cymbolTime < cymbolSustain ? stompColor : mixColor(map(cymbolTime,cymbolSustain,cymbolDecay,0,1000), stompColor, color);
      }

      leds.setPixel(i+column*NUM_LEDS, color);
    }
  }

  long delayTime = DELAY - (millis() - lastRender);
  if (delayTime > 0) {
    delay(delayTime);
  }
  lastRender = millis();
  
  leds.show();
}

void runInteractive() {
  readData();
  draw();
}

void loop() {
  if (state==INIT) {
    readData();
  } else if (state==ACTIVE) {
    runInteractive();
  }
}

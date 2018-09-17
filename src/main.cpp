#include <Arduino.h>
#include <OctoWS2811.h>

#define TOTAL_STRIPS 40
#define NUM_STRIPS 8
#define NUM_LEDS 50
#define CONTROLED_LEDS 400
#define DELAY 30

// states
#define INIT 0
#define ACTIVE 1
#define SCREEN_SAVER_1 2
#define SCREEN_SAVER_2 3

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

#define SCREEN_SAVER_COMMAND 5
#define SCREEN_SAVER_PARAMETER 0

#define CYCLE_ADV 1
// 2*pi/256
#define CYCLE_FREQ -0.0981747704

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

uint32_t logo[] = {0,0x40000,0,16,0x400000,0,256,0x3c000000,0,0x3f000,0x40000000,126,0xfc10000,0,0xfc04,0x80100000,31,0x1f00040,0,0x3f00,0xe8000000,7,0xfc2000,0x80000000,0xf80,0xf8020000,1,0x3f0008,0x200000,992,0x7e000080,0x2000000,0xffc00,0xfe000800,0x200000ff,0x1fffff00,0xff808000,0xffff,0xffffffe2,0xfff80003,0x1fffff,0xfffffff0,0xfff8007f,0x1ffffff,0xfffffffc,0xfffe05ff,0x11ffffff,0xffffffff,0xffffc0ff,0xffffff,0xffffffff,0xfffc007f,0x3fffff,0xfffffff0,0xffc0003f,0x1fffff,0xffffff00,0xfc00000f,0x7ffff,0xfffff000,0xc0000007,0x3ffff,0xffff0000,1,0x1fffc,0xfff00000,0,0x7fc0,0x3f000000,0,0x3c00,0x30000000,0,0x4000,0,1,0x40000,0,20,0x500000,0,320,0xd000000,0,0xfffff600,0xdfffffff,0xffffffff,0xffff7fff,0xffffffff,0xfffffffd,0xfff7ffff,0xffffffff,0xffffffdf,0xff7fffff,0xffffffff,0xfffffdff,0xf7ffffff,0xffffffff,0xffffdfff,0x7fffffff,0xffffffff,0xfffdffff,0xffffffff,0xfffffff7,0xffdfffff,0xffffffff,383,0x5000002,0x80000,0x1400,0x50000020,0x800000,0x4000,0x80000600,0x1c000001,0x60000,0xf800,0xf8000018,0xc0700007,0x3fffff,0xffffe1e0,0xc7fe3fff,0xffffffff,0xffff9fff,0x7fffffff,0xffffffff,0xfffdffff,0xffffffff,0xdffffff7,0xffcfffff,0xffff7fff,0xffffff3f,0xfcfffffc,0xfff3ffff,0xfffff3ff,0xc7ffff87,0xfe1fffff,0xffff1fff,0x3ffff03f,0x803ffffc,0xff07fff,0xfffc0000,960,0xf00ffc0,0,0x3c00,0xe0000000,1,0x3e0000,0,448,0,0x1fc0000,0,0x1ffff0,0xff000000,127,0x1ffe000,0,0x7fe,0x1fe00000,0,0x7e00,0xf0000000,1,0x378000,0,220,0x3600000,0,0xd80,0x36000000,0,0xd000,0x40000000,0xffffffff,0xfffdffff,0xffffffff,0xffffffc7,0xfc1fffff,0xffffffff,0xffffc07f,0x1ffffff,0xfffffffc,0xffc007ff,0x1fffffff,0xfffff800,0x80007fff,0xffffffff,0xfff80001,0x37ffff,0xffffff80,0xf80003df,0x3f7fffff,0xffff8000,0x3fdff,0xf7fffff0,63,0xffd000,0x40000000,0x3ff,0xffd0000,0,0x3ff4,0xffd00000,0,0x3ff60,0xfd800000,31,0x7ff600,0xd8000000,511,0x7ff6000,0,0x3ff8,0xffe00000,0,0x7ff80,0xfc000400,0x7800003f,0x1fff000,0xc007f000,0xe0000fff,0xfffe007f,0x7ffe001,0xf83ffff0,0xffc03fff,0x7fffffff,0xfffffe00,0xf000ffff,0xffffffff,0xffffc001,0x7ffff,0xfffffffe,0xffe0000f,0x1fffff,0xffffff00,0xf800001f,0x3fffff,0xffff8000,63,0x3ffff8,0xff000000,31,0,0,0,0,0,0,0,0xfffc0000,0xffffffff,0xfffffff7,0xffdfffff,0xffffffff,0xffffff7f,0xfdffffff,0xffffffff,0xfffff7ff,0xdfffffff,0xffffffff,0xffff7fff,0xffffffff,0xfffffffd,0xfff7ffff,0xffffffff,0xffffffdf,0xff7fffff,0x803ff003,0xc00ffdff,0xf7fe00ff,0x3ff003f,0xffdff8,0x7fe00ffc,0x3ff003ff,0xffdff80,0xfe00ffc0,0xff003ff7,0xffdff803,0xe00ffc00,0xf003ff7f,0xfdff803f,0xffc00f,0x3ff7fe,0xdff803ff,0xffc00ff,0x3ff7fe0,0xff803ff0,0xffc00ffd,0x3ff7fe00,0xf803ff00,0xfc00ffdf,0xff7fe00f,0x803ff003,0xffdff,0xf7fe0000,63,0xffdff8,0x7fe00000,510,0xff00,0,0,0,0,0,0,0,0xe00,0x3e000000,0,0xff00,0xff000000,3,0xfff00,0xff000000,63,0xffff00,0xff000000,0x3ff,0xfffff00,0xff800000,0x3fff,0xffffff80,0xff800000,0x1ffff,0xffffff80,0xff800001,0x1ffff,0xffffff80,0xffc00001,0x1ffff,0xffffffc0,0xffc00001,0xffff,0xffffffc0,0xffc00000,0xffff,0xffffffc0,0xffc00000,0xffff,0xffffffc0,0xff000000,0xffff,0xfffffc00,0xc0000007,0x7fffff,0xfffc0000,0xfff,0xffffffc0,0xfc000000,0xfffff,0xffffc000,255,0xffffffc,0xffc00000,0x1ffff,0xfffff800,0x8000001f,0x1ffffff,0xfff80000,0x1fff,0xffffff80,0xf8000001,0xfffff,0xffff8000,63,0xfffff8,0xff800000,0x3ff,0xffff000,0,0x3fff,0xfff00000,0,0x3ff00,0xf0000000,15,0x3f0000,0,240,0};

float distances[CONTROLED_LEDS];
float phases[CONTROLED_LEDS];
float drumDistances[CONTROLED_LEDS*5];

DMAMEM int displayMemory[NUM_LEDS*6];
int drawingMemory[NUM_LEDS*6];

const int config = WS2811_RGB | WS2811_800kHz;

OctoWS2811 leds(NUM_LEDS, displayMemory, drawingMemory, config);

int rainbowColors[180];

long lastRender = millis();
long initTime = 0;
long lastDrumHit[] = {0,0,0,0,0};
int32_t drumColors[] = {0xfe0000, 0xf9fe00, 0x00fe30, 0x0003fe, 0x9d00fe};
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

float logoOffset = 0.0;
float logoRate = 0.02;
float logoPhases[NUM_STRIPS];
float logoSizes[NUM_STRIPS];
float logoFreqs[NUM_STRIPS];

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
    logoPhases[i] = random(0,62830) / 10000.0;
    logoSizes[i] = random(-5000,8000) / 10000.0;
    logoFreqs[i] = 2.0 * M_PI / random(1200,8000);
  }
}

void readData() {
  while (Serial.available()) {
    int value = Serial.read();
    switch (command) {
      case NO_COMMAND:
        command = value;
        parameter = 0;
        break;
      case INIT_COMMAND:
        switch (parameter) {
          case OFFSET_PARAMETER:
            initTime = millis();
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
            state = ACTIVE;
          default:
            resetCommand(); break;
        }
        break;
      case DRUM_COMMAND:
        switch (parameter) {
          case DRUM_PARAMETER:
            lastDrumHit[value] = millis();
            state = ACTIVE;
          default:
            resetCommand(); break;
        }
        break;
      case PUMP_COMMAND:
        switch (parameter) {
          case PUMP_PARAMETER:
            pump = value;
            state = ACTIVE;
          default:
            resetCommand(); break;
        }
        break;
      case CYMBOLS_COMMAND:
        switch (parameter) {
          case CYMBOLS_PARAMETER:
            cymbol = value;
            lastCymbolHit = millis();
            state = ACTIVE;
          default:
            resetCommand(); break;
        }
        break;
      case SCREEN_SAVER_COMMAND:
        switch (value) {
          case 0:
            state = SCREEN_SAVER_1;
            resetCommand(); break;
          case 1:
            state = SCREEN_SAVER_2;
            resetCommand(); break;
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

  for (int i=0; i<CONTROLED_LEDS; i++) {
    leds.setPixel(i, 0x008800);
  }
  leds.show();

  delay(1000);

  for (int i=0; i<CONTROLED_LEDS; i++) {
    leds.setPixel(i, 0);
  }
  leds.show();

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

  for(uint8_t column=0; column<NUM_STRIPS; column++) {
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
        // dist = distances[xoffset + (int) (NUM_LEDS/2)];
        int rval = (int) (env*cos((cfreq*dist + R_PHASE)*cymbolTime));
        int gval = (int) (env*cos((cfreq*dist + G_PHASE)*cymbolTime));
        int bval = (int) (env*cos((cfreq*dist + B_PHASE)*cymbolTime));
        uint32_t r = rval < 0 ? 0xff0000 : 0x000000;
        uint32_t g = gval < 0 ? 0x00ff00 : 0x000000;
        uint32_t b = bval < 0 ? 0x0000ff : 0x000000;
        uint32_t stompColor = r | g | b;
        color = cymbolTime < cymbolSustain ? stompColor : mixColor(map(cymbolTime,cymbolSustain,cymbolDecay,0,1000), stompColor, color);
      }

      leds.setPixel(i+xoffset, color);
    }
  }

  long delayTime = DELAY - (millis() - lastRender);
  if (delayTime > 0) {
    delay(delayTime);
  }
  lastRender = millis();

  leds.show();
}

void drawLogo1() {
  for(uint8_t column=0; column<NUM_STRIPS; column++) {
    int x = column;
    int xoffset = x * NUM_LEDS;
    int coloffset = (3*(x+offset) + (int) (logoOffset))%245 * NUM_LEDS;
    logoOffset += logoRate;

    for(uint16_t i=0; i<NUM_LEDS; i++) {
      // leds.setPixel(i+xoffset, sin(logoFreqs[x]*millis() + logoPhases[x] + i*2*M_PI/NUM_LEDS) > logoSizes[x] ? 0xFFFFFF : 0x000000);
      int rrain = sin(logoFreqs[x]*millis() + logoPhases[x] + i*2*M_PI/NUM_LEDS) > logoSizes[x] ? 0x990000 : 0;
      int grain = sin(logoFreqs[x]*millis() + logoPhases[x] + i*2*M_PI/NUM_LEDS) > logoSizes[x] ? 0x00ff00 : 0;
      int brain = sin(-1*logoFreqs[x]*millis() + logoPhases[x] + i*2*M_PI/NUM_LEDS) > logoSizes[x] ? 0x0000ff : 0;
      // leds.setPixel(i+xoffset, rrain+grain+brain);

      int address = coloffset + i;
      uint32_t word = logo[(int) (address / 32)];
      int image = (word & (1<<(address%32))) ? 1 : 0;
      uint32_t color = image ? (0xff0000-(brain<<16)) + (0xff00-grain) + (0xff-(rrain>>16)) : rrain + grain + brain;
      leds.setPixel(i+xoffset, color);
    }
  }

  long delayTime = DELAY - (millis() - lastRender);
  if (delayTime > 0) {
    delay(delayTime);
  }
  lastRender = millis();

  leds.show();
}

void drawLogo2() {
  float fade = .9;

  for(uint8_t column=0; column<NUM_STRIPS; column++) {
    int x = column;
    int xoffset = x * NUM_LEDS;
    int coloffset = (3*(x+offset) + (int) (logoOffset))%245 * NUM_LEDS;
    logoOffset += logoRate;

    for(uint16_t i=0; i<NUM_LEDS; i++) {
      int address = coloffset + i;
      uint32_t word = logo[(int) (address / 32)];
      int image = (word & (1<<(address%32))) ? 1 : 0;
      uint32_t color = 0;
      if (image) {
        color = 0xFFFFFF;
      } else {
        if (i > 0) color += ((int) (fade*leds.getPixel(xoffset + i - 1))) & 0xFF0000;
        if (i < NUM_LEDS-1) color += (int) (fade*((leds.getPixel(xoffset + i + 1)&0xFF)));
      }

      leds.setPixel(xoffset + i, color);
    }
  }

  long delayTime = DELAY - (millis() - lastRender);
  if (delayTime > 0) {
    delay(delayTime);
  }
  lastRender = millis();

  leds.show();
}

void drawTest() {
  for(uint8_t column=0; column<NUM_STRIPS; column++) {
    int x = column;
    int xoffset = x * NUM_LEDS;

    for(uint16_t i=0; i<NUM_LEDS; i++) {
      leds.setPixel(xoffset + i, ((millis() - initTime) / 1000)%TOTAL_STRIPS == x+offset ? 0xFF0000 : 0x003300);
    }
  }

  long delayTime = DELAY - (millis() - lastRender);
  if (delayTime > 0) {
    delay(delayTime);
  }
  lastRender = millis();

  leds.show();
}

void loop() {
  readData();
  if (state==INIT) {
    delay(1);
  } else if (state==ACTIVE) {
    draw();
  } else if (state==SCREEN_SAVER_1) {
    drawLogo1();
  } else if (state==SCREEN_SAVER_2) {
    drawLogo2();
  }
}

#include <Arduino.h>
#include <OctoWS2811.h>

#define BAND_WIDTH 1.0

#define INITIALIZING 0
#define LISTENING 1
#define DRAWING 2
#define STOPPED 3

const int ledsPerStrip = 10;

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

int rainbowColors[180];
int currentColor = 0;
float cycleWidth = BAND_WIDTH * M_PI / 2.0;
long spitStart = 0;

int state = LISTENING;

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}

int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness)
{
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

void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  for (int i=0; i<180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 10;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }
  digitalWrite(1, LOW);
  leds.begin();

  Serial.begin(9600);
}

void init() {
  int command = Serial.read();
  if (command != 0) {
    return;
  }
  state = LISTENING;
}

void listen() {
  int command = Serial.read();
  if (command == 1) {
    state = DRAWING;
    spitStart = millis();
  }
  else if (command == 2)
    state = STOPPED;
}

void draw() {
  float centerx = 3.5;
  float centery = (ledsPerStrip-1.0) / 2.0;
  long deltat = millis() - spitStart;
  float pulse = pow(deltat * .002, 1.8) + 1.0;
  if (pulse > 10.0) {
    state = LISTENING;
    return;
  }

  int color = rainbowColors[currentColor];
  currentColor = (currentColor+1) % 180;
  int r = color >> 16;
  int g = (color & 0xff00) >> 8;
  int b = color & 0xff;

  for (int x=0; x < 8; x++) {
    for (int y=0; y < ledsPerStrip; y++) {
      float dx = x-centerx, dy= y-centery;
      float doff = sqrt(dx*dx + dy*dy) - pulse;
      if (abs(doff) <= cycleWidth) {
        float bright = abs(cos(doff/BAND_WIDTH));
        leds.setPixel(x*ledsPerStrip + y,
          (((int) (bright*r)) << 16)
          + (((int) (bright*g)) << 8)
          + ((int) (bright*b))
        );
      } else {
        leds.setPixel(x*ledsPerStrip + y, 0);
      }
    }
  }
  leds.show();
  delay(5);
}

void loop() {
  switch (state) {
    case INITIALIZING:
      init();
      break;
    case LISTENING:
      listen();
      break;
    case DRAWING:
      draw();
      break;
    default:
      delay(10);
  }
}

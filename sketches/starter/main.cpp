#include <Adafruit_NeoPixel.h>

#define BATTERY_PIN A7
#define LED_RED_PIN 13
#define NEOPIXEL_BRIGHTNESS 15
#define NEOPIXEL_PIN 6
#define NEOPIXEL_NUMPIXELS 32

float measuredVoltageMin = 0;
float measuredVoltageMax = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_NUMPIXELS, NEOPIXEL_PIN, NEO_GRB | NEO_KHZ800);

void setup()
{
	Serial.begin(115200);
	Serial.println("\r\nFeather ready...");

	pinMode(BATTERY_PIN, INPUT);
	pinMode(LED_RED_PIN, OUTPUT);

	strip.begin();
	strip.setBrightness(NEOPIXEL_BRIGHTNESS);
	strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
	for (uint16_t i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		delay(wait);
	}
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// alternate between two colors on even and odd LEDs
void alternateColor(uint32_t c1, uint32_t c2, uint8_t wait)
{
	for (uint16_t i = 0; i < strip.numPixels(); i++) {
		if (i % 2 == 0) { // set even LED to color 1
			strip.setPixelColor(i, c1);
		} else { // set odd LED to color 2
			strip.setPixelColor(i, c2);
		}
	}

	strip.show(); // apply the colors
	delay(wait);

	for (uint16_t i = 0; i < strip.numPixels(); i++) {
		if (i % 2 == 0) { // set even LED to color 2
			strip.setPixelColor(i, c2);
		} else { // set odd LED to color 1
			strip.setPixelColor(i, c1);
		}
	}

	strip.show(); // apply the colors
	delay(wait);
}

void clearStrip()
{
	for (int i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, strip.Color(0, 0, 0)); // change RGB color value here
	}
	strip.show();
}

// gradually fill up the strip with random colors
void randomColorFill(uint8_t wait)
{
	clearStrip();

	for (uint16_t i = 0; i < strip.numPixels(); i++) { // iterate over every LED of the strip
		int r = random(0, 255);			   // generate a random color
		int g = random(0, 255);
		int b = random(0, 255);

		for (uint16_t j = 0; j < strip.numPixels() - i;
		     j++) { // iterate over every LED of the strip, that hasn't lit up yet
			strip.setPixelColor(j - 1, strip.Color(0, 0, 0)); // turn previous LED off
			strip.setPixelColor(j, strip.Color(r, g, b));     // turn current LED on
			strip.show();					  // apply the colors
			delay(wait);
		}
	}
}

// pick a random LED to light up until entire strip is lit
void randomPositionFill(uint32_t c, uint16_t wait)
{
	clearStrip();

	int used[strip.numPixels()]; // array to keep track of lit LEDs
	int lights = 0;		     // counter

	for (int i = 0; i < strip.numPixels(); i++) { // fill array with 0
		used[i] = 0;
	}

	while (lights < strip.numPixels() - 1) {
		int j = random(0, strip.numPixels() - 1); // pick a random LED
		if (used[j] != 1) {			  // if LED not already lit, proceed
			strip.setPixelColor(j, c);
			used[j] = 1; // update array to remember it is lit
			lights++;
			strip.show(); // display
			delay(wait);
		}
	}
}

// Light up the strip starting from the middle
void middleFill(uint32_t c, uint8_t wait, bool reverse)
{
	clearStrip();
	delay(wait);

	for (uint16_t i = 0; i < (strip.numPixels() / 2); i++) { // start from the middle, lighting an LED on each side
		strip.setPixelColor(strip.numPixels() / 2 + i, c);
		strip.setPixelColor(strip.numPixels() / 2 - i, c);
		strip.show();
		delay(wait);
	}
	strip.setPixelColor(0, c);
	strip.show();
	delay(wait);

	if (!reverse) {
		clearStrip();
		return;
	}

	for (uint16_t i = 0; i < (strip.numPixels() / 2); i++) { // reverse
		strip.setPixelColor(i, strip.Color(0, 0, 0));
		strip.setPixelColor(strip.numPixels() - i, strip.Color(0, 0, 0));
		strip.show();
		delay(wait);
	}
	clearStrip();
}

// Light up the strip starting from the sides
void sideFill(uint32_t c, uint8_t wait)
{
	clearStrip();
	delay(wait);

	for (uint16_t i = 0; i < (strip.numPixels() / 2); i++) { // fill strip from sides to middle
		strip.setPixelColor(i, c);
		strip.setPixelColor(strip.numPixels() - i, c);
		strip.show();
		delay(wait);
	}

	strip.setPixelColor(strip.numPixels() / 2, c);
	strip.show();
	delay(wait);

	for (uint16_t i = 0; i < (strip.numPixels() / 2); i++) { // reverse
		strip.setPixelColor(strip.numPixels() / 2 + i, strip.Color(0, 0, 0));
		strip.setPixelColor(strip.numPixels() / 2 - i, strip.Color(0, 0, 0));
		strip.show();
		delay(wait);
	}
	clearStrip();
}

void setPixel(int Pixel, byte red, byte green, byte blue)
{
	strip.setPixelColor(Pixel, strip.Color(red, green, blue));
}

void setAll(byte red, byte green, byte blue)
{
	for (int i = 0; i < strip.numPixels(); i++) {
		setPixel(i, red, green, blue);
	}
	strip.show();
}

void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, boolean OnlyOne)
{
	setAll(0, 0, 0);

	for (int i = 0; i < Count; i++) {
		setPixel(random(strip.numPixels()), red, green, blue);
		strip.show();
		delay(SpeedDelay);
		if (OnlyOne) {
			setAll(0, 0, 0);
		}
	}

	delay(SpeedDelay);
}

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne)
{
	setAll(0, 0, 0);

	for (int i = 0; i < Count; i++) {
		setPixel(random(strip.numPixels()), random(0, 255), random(0, 255), random(0, 255));
		strip.show();
		delay(SpeedDelay);
		if (OnlyOne) {
			setAll(0, 0, 0);
		}
	}

	delay(SpeedDelay);
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay)
{
	int Pixel = random(strip.numPixels());
	setPixel(Pixel, red, green, blue);
	strip.show();
	delay(SpeedDelay);
	setPixel(Pixel, 0, 0, 0);
}

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay)
{
	setAll(red, green, blue);

	int Pixel = random(strip.numPixels());
	setPixel(Pixel, 0xff, 0xff, 0xff);
	strip.show();
	delay(SparkleDelay);
	setPixel(Pixel, red, green, blue);
	strip.show();
	delay(SpeedDelay);
}

void RGBLoop()
{
	for (int j = 0; j < 3; j++) {
		// Fade IN
		for (int k = 0; k < 256; k++) {
			switch (j) {
			case 0:
				setAll(k, 0, 0);
				break;
			case 1:
				setAll(0, k, 0);
				break;
			case 2:
				setAll(0, 0, k);
				break;
			}
			strip.show();
			delay(3);
		}
		// Fade OUT
		for (int k = 255; k >= 0; k--) {
			switch (j) {
			case 0:
				setAll(k, 0, 0);
				break;
			case 1:
				setAll(0, k, 0);
				break;
			case 2:
				setAll(0, 0, k);
				break;
			}
			strip.show();
			delay(3);
		}
	}
}

void FadeIn(byte red, byte green, byte blue, int count)
{
	float r, g, b;

	do {
		for (int k = 0; k < 256; k = k + 1) {
			r = (k / 256.0) * red;
			g = (k / 256.0) * green;
			b = (k / 256.0) * blue;
			setAll(r, g, b);
			strip.show();
			delay(2);
		}
		setAll(0, 0, 0);
		strip.show();
		delay(100);
	} while (--count);
}

void FadeInOut(byte red, byte green, byte blue)
{
	float r, g, b;

	for (int k = 0; k < 256; k = k + 1) {
		r = (k / 256.0) * red;
		g = (k / 256.0) * green;
		b = (k / 256.0) * blue;
		setAll(r, g, b);
		strip.show();
	}

	for (int k = 255; k >= 0; k = k - 2) {
		r = (k / 256.0) * red;
		g = (k / 256.0) * green;
		b = (k / 256.0) * blue;
		setAll(r, g, b);
		strip.show();
	}
}

void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause)
{
	for (int j = 0; j < StrobeCount; j++) {
		setAll(red, green, blue);
		strip.show();
		delay(FlashDelay);
		setAll(0, 0, 0);
		strip.show();
		delay(FlashDelay);
	}

	delay(EndPause);
}

void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{

	for (int i = 0; i < strip.numPixels() - EyeSize - 2; i++) {
		setAll(0, 0, 0);
		setPixel(i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(i + j, red, green, blue);
		}
		setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		strip.show();
		delay(SpeedDelay);
	}

	delay(ReturnDelay);

	for (int i = strip.numPixels() - EyeSize - 2; i > 0; i--) {
		setAll(0, 0, 0);
		setPixel(i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(i + j, red, green, blue);
		}
		setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		strip.show();
		delay(SpeedDelay);
	}

	delay(ReturnDelay);
}

void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
	for (int i = ((strip.numPixels() - EyeSize) / 2); i >= 0; i--) {
		setAll(0, 0, 0);

		setPixel(i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(i + j, red, green, blue);
		}
		setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);

		setPixel(strip.numPixels() - i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(strip.numPixels() - i - j, red, green, blue);
		}
		setPixel(strip.numPixels() - i - EyeSize - 1, red / 10, green / 10, blue / 10);

		strip.show();
		delay(SpeedDelay);
	}
	delay(ReturnDelay);
}

void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
	for (int i = 0; i <= ((strip.numPixels() - EyeSize) / 2); i++) {
		setAll(0, 0, 0);

		setPixel(i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(i + j, red, green, blue);
		}
		setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);

		setPixel(strip.numPixels() - i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(strip.numPixels() - i - j, red, green, blue);
		}
		setPixel(strip.numPixels() - i - EyeSize - 1, red / 10, green / 10, blue / 10);

		strip.show();
		delay(SpeedDelay);
	}
	delay(ReturnDelay);
}

void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
	for (int i = 0; i < strip.numPixels() - EyeSize - 2; i++) {
		setAll(0, 0, 0);
		setPixel(i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(i + j, red, green, blue);
		}
		setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		strip.show();
		delay(SpeedDelay);
	}
	delay(ReturnDelay);
}

void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
	for (int i = strip.numPixels() - EyeSize - 2; i > 0; i--) {
		setAll(0, 0, 0);
		setPixel(i, red / 10, green / 10, blue / 10);
		for (int j = 1; j <= EyeSize; j++) {
			setPixel(i + j, red, green, blue);
		}
		setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
		strip.show();
		delay(SpeedDelay);
	}
	delay(ReturnDelay);
}

void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
	RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay)
{
	int Position = 0;

	for (int i = 0; i < strip.numPixels() * 2; i++) {
		Position++; // = 0; //Position + Rate;
		for (int i = 0; i < strip.numPixels(); i++) {
			// sine wave, 3 offset waves make a rainbow!
			// float level = sin(i+Position) * 127 + 128;
			// setPixel(i,level,0,0);
			// float level = sin(i+Position) * 127 + 128;
			setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
				 ((sin(i + Position) * 127 + 128) / 255) * green,
				 ((sin(i + Position) * 127 + 128) / 255) * blue);
		}

		strip.show();
		delay(WaveDelay);
	}
}

void rainbow(uint8_t wait)
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) {
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i + j) & 255));
		}
		strip.show();
		delay(wait);
	}
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
	uint16_t i, j;

	for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
		}
		strip.show();
		delay(wait);
	}
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
	for (int j = 0; j < 10; j++) { // do 10 cycles of chasing
		for (int q = 0; q < 3; q++) {
			for (int i = 0; i < strip.numPixels(); i = i + 3) {
				strip.setPixelColor(i + q, c); // turn every third pixel on
			}
			strip.show();

			delay(wait);

			for (int i = 0; i < strip.numPixels(); i = i + 3) {
				strip.setPixelColor(i + q, 0); // turn every third pixel off
			}
		}
	}
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
	for (int j = 0; j < 256; j++) { // cycle all 256 colors in the wheel
		for (int q = 0; q < 3; q++) {
			for (int i = 0; i < strip.numPixels(); i = i + 3) {
				strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
			}
			strip.show();

			delay(wait);

			for (int i = 0; i < strip.numPixels(); i = i + 3) {
				strip.setPixelColor(i + q, 0); // turn every third pixel off
			}
		}
	}
}

void rainbowStrip()
{

	int i = 0;
	while (i < strip.numPixels() * 4) {
		strip.setPixelColor(i % strip.numPixels(), strip.Color(255, 255, 255)); // change RGB color value here
		strip.setPixelColor((i + 1) % strip.numPixels(),
				    strip.Color(255, 5, 180));				  // change RGB color value here
		strip.setPixelColor((i + 2) % strip.numPixels(), strip.Color(255, 0, 0)); // change RGB color value here
		strip.setPixelColor((i + 3) % strip.numPixels(),
				    strip.Color(255, 150, 0)); // change RGB color value here
		strip.setPixelColor((i + 4) % strip.numPixels(),
				    strip.Color(255, 255, 5));				  // change RGB color value here
		strip.setPixelColor((i + 5) % strip.numPixels(), strip.Color(0, 255, 0)); // change RGB color value here
		strip.setPixelColor((i + 6) % strip.numPixels(), strip.Color(0, 0, 255)); // change RGB color value here
		strip.setPixelColor((i + 7) % strip.numPixels(),
				    strip.Color(135, 10, 215)); // change RGB color value here
		strip.setPixelColor((i + 8) % strip.numPixels(),
				    strip.Color(255, 255, 255)); // change RGB color value here
		strip.setPixelColor((i + 9) % strip.numPixels(),
				    strip.Color(255, 5, 180)); // change RGB color value here
		strip.setPixelColor((i + 10) % strip.numPixels(), strip.Color(255, 0, 0)); // change RGB color value
											   // here
		strip.setPixelColor((i + 11) % strip.numPixels(),
				    strip.Color(255, 150, 0)); // change RGB color value here
		strip.setPixelColor((i + 12) % strip.numPixels(),
				    strip.Color(255, 255, 5)); // change RGB color value here
		strip.setPixelColor((i + 13) % strip.numPixels(), strip.Color(0, 255, 0)); // change RGB color value
											   // here
		strip.setPixelColor((i + 14) % strip.numPixels(), strip.Color(0, 0, 255)); // change RGB color value
											   // here
		strip.setPixelColor((i + 15) % strip.numPixels(),
				    strip.Color(135, 10, 215)); // change RGB color value here
		strip.setPixelColor((i + 16) % strip.numPixels(),
				    strip.Color(255, 255, 255)); // change RGB color value here
		strip.setPixelColor((i + 17) % strip.numPixels(),
				    strip.Color(255, 5, 180)); // change RGB color value here
		strip.setPixelColor((i + 18) % strip.numPixels(), strip.Color(255, 0, 0)); // change RGB color value
											   // here
		strip.setPixelColor((i + 19) % strip.numPixels(),
				    strip.Color(255, 150, 0)); // change RGB color value here
		strip.setPixelColor((i + 20) % strip.numPixels(),
				    strip.Color(255, 255, 5)); // change RGB color value here
		strip.setPixelColor((i + 21) % strip.numPixels(), strip.Color(0, 255, 0)); // change RGB color value
											   // here
		strip.setPixelColor((i + 22) % strip.numPixels(), strip.Color(0, 0, 255)); // change RGB color value
											   // here
		strip.setPixelColor((i + 23) % strip.numPixels(),
				    strip.Color(135, 10, 215)); // change RGB color value here
		strip.setPixelColor((i + 24) % strip.numPixels(),
				    strip.Color(255, 255, 255)); // change RGB color value here
		strip.setPixelColor((i + 25) % strip.numPixels(),
				    strip.Color(255, 5, 180)); // change RGB color value here
		strip.setPixelColor((i + 26) % strip.numPixels(), strip.Color(255, 0, 0)); // change RGB color value
											   // here
		strip.setPixelColor((i + 27) % strip.numPixels(),
				    strip.Color(255, 150, 0)); // change RGB color value here
		strip.setPixelColor((i + 28) % strip.numPixels(),
				    strip.Color(255, 255, 5)); // change RGB color value here
		strip.setPixelColor((i + 29) % strip.numPixels(), strip.Color(0, 255, 0)); // change RGB color value
											   // here
		strip.setPixelColor((i + 30) % strip.numPixels(), strip.Color(0, 0, 255)); // change RGB color value
											   // here
		strip.setPixelColor((i + 31) % strip.numPixels(),
				    strip.Color(135, 10, 215)); // change RGB color value here
		i++;
		strip.show();
		delay(75);
	}
}

void colorWipe2(byte red, byte green, byte blue, int SpeedDelay)
{
	for (uint16_t i = 0; i < strip.numPixels(); i++) {
		setPixel(i, red, green, blue);
		strip.show();
		delay(SpeedDelay);
	}
}

void startShow(int i)
{
	switch (i) {
	case 1:
		colorWipe(strip.Color(0, 0, 0), 50); // Black/off
		break;
	case 2:
		theaterChase(strip.Color(127, 127, 127), 50); // White
		break;
	case 3:
		theaterChase(strip.Color(127, 0, 0), 50); // Red
		break;
	case 4:
		theaterChase(strip.Color(0, 0, 127), 50); // Blue
		break;
	case 5:
		theaterChaseRainbow(50);
		break;
	case 6:
		rainbow(20);
		break;
	case 7:
		// rainbowCycle(20);
		break;
	case 8:
		rainbowStrip();
		break;
	case 9:
		randomColorFill(25);
		break;
	case 10:
		randomPositionFill(strip.Color(255, 255, 255), 500);
		break;
	case 11:
		middleFill(strip.Color(0, 255, 0), 100, true);
		break;
	case 12:
		sideFill(strip.Color(255, 0, 0), 100);
		break;
	case 13:
		RGBLoop();
		break;
	case 14:
		// FadeInOut(0xff, 0x00, 0x00); // red
		break;
	case 15:
		// FadeInOut(0xff, 0xff, 0xff); // white
		break;
	case 16:
		// FadeInOut(0x00, 0x00, 0xff); // blue
		break;
	case 17:
		// Strobe(0xff, 0x77, 0x00, 10, 100, 1000);
		break;
	case 18:
		CylonBounce(0xff, 0, 0, 3, 35, 75);
		break;
	case 19:
		NewKITT(0xff, 0, 0, 4, 35, 75);
		break;
	case 20:
		RunningLights(0xff, 0, 0, 50); // red
		break;
	case 21:
		RunningLights(0xff, 0xff, 0xff, 50); // white
		break;
	case 22:
		RunningLights(0, 0, 0xff, 50); // blue
		break;
	case 23:
		RunningLights(0xff, 0, 0xff, 50); // green
		break;
	}
}

void AlternatingColorHalf(uint32_t c, uint8_t count, uint8_t wait)
{
	setAll(0, 0, 0);
	strip.show();

	do {
		for (int led = 0; led < 32; led++) {
			int pos = (32-(led+1)) % 8;
			if (pos >= 4)
				strip.setPixelColor(led, c);
		}

		strip.show();
		delay(wait);

		setAll(0, 0, 0);
		strip.show();
		delay(wait*2);

		for (int led = 0; led < 32; led++) {
			int pos = (32-(led+1)) % 8;
			if (pos <= 3)
				strip.setPixelColor(led, c);
		}

		strip.show();
		delay(wait);

		setAll(0, 0, 0);
		strip.show();
		delay(wait*2);
	} while (--count);
}

void loop()
{
	digitalWrite(LED_RED_PIN, HIGH);

	float measuredVoltage = analogRead(BATTERY_PIN);

	measuredVoltage *= 2;    // we divided by 2, so multiply back
	measuredVoltage *= 3.3;  // Multiply by 3.3V, our reference voltage
	measuredVoltage /= 1024; // convert to voltage

	if (measuredVoltage <= 0)
		return;

	if (measuredVoltage > measuredVoltageMax)
		measuredVoltageMax = measuredVoltage;
	if (measuredVoltageMin == 0)
		measuredVoltageMin = measuredVoltage;

	if (measuredVoltageMin > measuredVoltage)
		measuredVoltageMin = measuredVoltage;

	Serial.print("Battery: ");
	Serial.print(measuredVoltage);
	Serial.print(" Max: ");
	Serial.print(measuredVoltageMax);
	Serial.print(" Min: ");
	Serial.println(measuredVoltageMin);

	clearStrip();
	middleFill(strip.Color(0, 0, 255), 100, false);
	// FadeIn(0, 0, 255, 1);
	for (int i = 0; i < 3; i++) {
		Strobe(0, 0, 255, 3, 100, 0);
		AlternatingColorHalf(strip.Color(0, 0, 255), 5, 70);
		Strobe(0, 0, 255, 3, 100, 0);
		AlternatingColorHalf(strip.Color(255, 0, 0), 5, 70);

		Strobe(255, 0, 0, 3, 100, 0);
		AlternatingColorHalf(strip.Color(255, 0, 0), 5, 70);
		Strobe(255, 0, 0, 3, 100, 0);
		AlternatingColorHalf(strip.Color(255, 0, 0), 5, 70);
	}

	clearStrip();
	delay(50);

	for (int i = 0; i <= 22; i++) {
		startShow(i);
	}

	digitalWrite(LED_RED_PIN, LOW);
}

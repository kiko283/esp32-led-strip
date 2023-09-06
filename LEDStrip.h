#include <NeoPixelBus.h>

// Modes supported:
enum  mode { CONTINUOUS, BLINKING, COLOR_WIPE, THEATER_CHASE, SCANNER, FADE, RAINBOW_CYCLE };
// Mode directions supported:
enum  direction { FORWARD, REVERSE };

// Custom Class - derived from the NeoPixelBus class
class LEDStrip : public NeoPixelBus <NeoGrbFeature, Neo800KbpsMethod> {
  public:
    // Member Variables:
    mode ActiveMode;          // which pattern is running
    direction Direction;      // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    // uint32_t Color1, Color2;  // what colors are in use
    // uint32_t RainbowColors[7] = { 0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF, 0xFF0000};

    RgbColor Color1 = RgbColor(0, 0, 0), Color2 = RgbColor(0, 0, 0);
    RgbColor RainbowColors[7] = {
      RgbColor(0x00, 0xFF, 0xFF),
      RgbColor(0x00, 0x00, 0xFF),
      RgbColor(0xFF, 0x00, 0xFF),
      RgbColor(0xFF, 0x00, 0x00),
      RgbColor(0xFF, 0xFF, 0x00),
      RgbColor(0x00, 0xFF, 0x00),
      RgbColor(0x00, 0xFF, 0xFF)
    };
    RgbColor RainbowFadeColor1 = RgbColor(0, 0, 0), RainbowFadeColor2 = RgbColor(0, 0, 0);

    uint8_t RainbowScale = 0xFF;
    uint8_t RainbowIndex = 0;
    uint16_t TotalSteps;      // total number of steps in the pattern
    uint16_t Index;           // current step within the pattern
    uint8_t EveryXth;         // theater chase - show color on every Xth led
    bool RainbowOn;           // is rainbow used as color

    void (*OnComplete)();     // Callback on completion of pattern

    // Constructor (main) - calls base-class constructor to initialize strip
    LEDStrip(uint16_t pixels, uint8_t pin)
      : NeoPixelBus <NeoGrbFeature, Neo800KbpsMethod> (pixels, pin) {
      OnComplete = NULL;
    }

    // Constructor with callback on pattern cycle end
    LEDStrip(uint16_t pixels, uint8_t pin, void (*callback)())
      : LEDStrip(pixels, pin) {
      OnComplete = callback;
    }

    // HACKISH BUT WORKS, LET IT BE FOR NOW

    void SetColor1(uint32_t color) {
      uint8_t r = (color >> 16) & 0xFF;
      uint8_t g = (color >> 8) & 0xFF;
      uint8_t b = color & 0xFF;
      Color1 = RgbColor(r, g, b);
    }
    uint32_t GetColor1() { return (Color1.R << 16) + (Color1.G << 8) + Color1.B; }

    void SetColor2(uint32_t color) {
      uint8_t r = (color >> 16) & 0xFF;
      uint8_t g = (color >> 8) & 0xFF;
      uint8_t b = color & 0xFF;
      Color2 = RgbColor(r, g, b);
    }
    uint32_t GetColor2() { return (Color2.R << 16) + (Color2.G << 8) + Color2.B; }

    // TODO Main app should work with this

    // RgbColor GetColor1() { return Color1; }
    // void SetColor1(RgbColor color) { Color1 = color; }

    // RgbColor GetColor2() { return Color2; }
    // void SetColor2(RgbColor color) { Color2 = color; }

    bool IsRainbow() { return RainbowOn; }
    void SetRainbow(bool rb) { RainbowOn = rb; }

    uint8_t GetRainbowScale() { return RainbowScale; }
    void SetRainbowScale(uint8_t rbs) { RainbowScale = rbs; }

    mode GetActiveMode() { return ActiveMode; }
    void SetActiveMode(mode am) { ActiveMode = am; }

    // Update the pattern
    void Update(unsigned long currentMillis) {
      if ((currentMillis - lastUpdate) > Interval) { // time to update
        lastUpdate = currentMillis;
        switch (ActiveMode) {
          case CONTINUOUS:
            ContinuousUpdate();
            break;
          case BLINKING:
            BlinkingUpdate();
            break;
          case COLOR_WIPE:
            ColorWipeUpdate();
            break;
          case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
          case SCANNER:
            ScannerUpdate();
            break;
          case FADE:
            FadeUpdate();
            break;
          case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
          default:
            ClearColor();
            break;
        }
      }
    }

    // Increment the Index and reset at the end
    void Increment() {
      if (Direction == FORWARD) {
        Index++;
        if (Index >= TotalSteps) {
          Index = 0;
          if (OnComplete != NULL) {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else {
        --Index;
        if (Index <= 0) {
          Index = TotalSteps - 1;
          if (OnComplete != NULL) {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }

    // Reverse pattern direction
    void Reverse() {
      if (Direction == FORWARD) {
        Direction = REVERSE;
        Index = TotalSteps - 1;
      }
      else {
        Direction = FORWARD;
        Index = 0;
      }
    }

    // Initialize for Continuous
    void Continuous(uint8_t interval, direction dir = FORWARD) {
      ActiveMode = CONTINUOUS;
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update Continuous Mode
    void ContinuousUpdate() {
      if (RainbowOn) Rainbow(0);
      else SetColor(Color1);
      Show();
      Increment();
    }

    // Initialize for Blinking
    void Blinking(uint16_t steps, uint8_t interval, direction dir = FORWARD) {
      ActiveMode = BLINKING;
      Interval = interval;
      TotalSteps = steps;
      Index = 0;
      Direction = dir;
    }

    // Update Blinking Mode
    void BlinkingUpdate() {
      if (RainbowOn) {
        if ((Index / (TotalSteps / 2)) % 2 == 0) {
          Rainbow(0);
        }
        else {
          SetColor(RgbColor(0, 0, 0));
        }
      }
      else {
        SetColor((Index / (TotalSteps / 2)) % 2 == 0 ? Color1 : Color2);
      }
      Show();
      Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint8_t interval, direction dir = FORWARD) {
      ActiveMode = COLOR_WIPE;
      Interval = interval;
      TotalSteps = PixelCount() + 1;
      Index = 0;
      Direction = dir;
    }

    // Update ColorWipe Mode
    void ColorWipeUpdate() {
      if (Index == 0) ClearColor();
      else {
        if (RainbowOn) SetPixelColor(Index - 1, Wheel((Index * 256 / (PixelCount() - 1)) & 255));
        else SetPixelColor(Index - 1, Color1);
        Show();
      }
      Increment();
    }

    // Initialize for a TheaterChase
    void TheaterChase(uint8_t interval, uint8_t everyXth = 8, direction dir = REVERSE) {
      ActiveMode = THEATER_CHASE;
      Interval = interval;
      TotalSteps = PixelCount();
      Index = 0;
      Direction = dir;
      EveryXth = everyXth;
    }

    // Update TheaterChase Mode
    void TheaterChaseUpdate() {
      for (int i = 0; i < PixelCount(); i++) {
        if ((i + Index) % EveryXth == 0) {
          if (RainbowOn) SetPixelColor(i, Wheel((i * 256 / (PixelCount() - 1)) & 255));
          else SetPixelColor(i, Color1);
        }
        else {
          if (RainbowOn) SetPixelColor(i, RgbColor(0, 0, 0));
          else SetPixelColor(i, DimColor(Color2));
        }
      }
      Show();
      Increment();
    }

    // Initialize for a Scanner
    void Scanner(uint8_t interval) {
      ActiveMode = SCANNER;
      Interval = interval;
      TotalSteps = (PixelCount() - 1) * 2;
      Index = 0;
      Direction = FORWARD;
    }

    // Update Scanner Mode
    void ScannerUpdate() {
      for (int i = 0; i < PixelCount(); i++) {
        if (i == Index) { // Scan Pixel to the right
          if (RainbowOn) SetPixelColor(i, Wheel((i * 256 / (PixelCount() - 1)) & 255));
          else SetPixelColor(i, Color1);
        }
        else if (i == TotalSteps - Index) { // Scan Pixel to the left
          if (RainbowOn) SetPixelColor(i, Wheel((i * 256 / (PixelCount() - 1)) & 255));
          else SetPixelColor(i, Color1);
        }
        else { // Fading tail
          SetPixelColor(i, DimColor(GetPixelColor(i)));
        }
      }
      Show();
      Increment();
    }

    // Initialize for a Fade
    void Fade(uint16_t steps, uint8_t interval, direction dir = FORWARD) {
      ActiveMode = FADE;
      Interval = interval;
      TotalSteps = steps;
      Index = 0;
      Direction = dir;
    }

    // Update Fade Mode
    void FadeUpdate() {
      // Calculate linear interpolation between Color1 and Color2
      // Optimise order of operations to minimize truncation error
      if (RainbowOn) {
        Direction = FORWARD;
        RainbowFadeColor1 = RainbowColors[RainbowIndex];
        RainbowFadeColor2 = RainbowColors[RainbowIndex + 1];
        uint8_t red =   ScaledValue(((RainbowFadeColor1.R * (TotalSteps - Index)) + (RainbowFadeColor2.R * Index)) / TotalSteps);
        uint8_t green = ScaledValue(((RainbowFadeColor1.G * (TotalSteps - Index)) + (RainbowFadeColor2.G * Index)) / TotalSteps);
        uint8_t blue =  ScaledValue(((RainbowFadeColor1.B * (TotalSteps - Index)) + (RainbowFadeColor2.B * Index)) / TotalSteps);
        SetColor(RgbColor(red, green, blue));

        Show();
        if (Index >= TotalSteps - 1) {
          RainbowIndex++;
          if (RainbowIndex >= 6) RainbowIndex = 0;
        }
        Increment();
      }
      else {
        uint8_t red = ((Color1.R * (TotalSteps - Index)) + (Color2.R * Index)) / TotalSteps;
        uint8_t green = ((Color1.G * (TotalSteps - Index)) + (Color2.G * Index)) / TotalSteps;
        uint8_t blue = ((Color1.B * (TotalSteps - Index)) + (Color2.B * Index)) / TotalSteps;
        SetColor(RgbColor(red, green, blue));

        Show();
        Increment();
        if (Index >= TotalSteps - 1) Reverse();
        else if (Index <= 0) Reverse();
      }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD) {
      ActiveMode = RAINBOW_CYCLE;
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update RainbowCycle Mode
    void RainbowCycleUpdate() {
      Rainbow(Index);
      Show();
      Increment();
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    RgbColor DimColor(RgbColor color) {
      // Shift R, G and B components one bit to the right
      return RgbColor(color.R >> 1, color.G >> 1, color.B >> 1);
    }

    // Set all pixels to a color (synchronously)
    void SetColor(RgbColor color) {
      for (int i = 0; i < PixelCount(); i++) {
        SetPixelColor(i, color);
      }
    }

    // Rainbow Color
    void Rainbow(uint16_t index) {
      for (int i = 0; i < PixelCount(); i++) {
        SetPixelColor(i, Wheel(((i * 256 / (PixelCount() - 1)) + index) & 255));
      }
    }

    // Set all pixels to off
    void ClearColor() {
      for (int i = 0; i < PixelCount(); i++) {
        SetPixelColor(i, RgbColor(0, 0, 0));
      }
      Show();
    }

    // Returns the scaled value of a color component for Rainbow
    uint8_t ScaledValue(uint8_t colorComponent) {
      return (colorComponent * RainbowScale) / 255;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    RgbColor Wheel(uint8_t wheelPos) {
      // TODO RainbowScale
      wheelPos = 255 - wheelPos;
      if (wheelPos < 85) {
        return RgbColor(ScaledValue(255 - wheelPos * 3), 0, ScaledValue(wheelPos * 3));
      }
      else if (wheelPos < 170) {
        wheelPos -= 85;
        return RgbColor(0, ScaledValue(wheelPos * 3), ScaledValue(255 - wheelPos * 3));
      }
      else {
        wheelPos -= 170;
        return RgbColor(ScaledValue(wheelPos * 3), ScaledValue(255 - wheelPos * 3), 0);
      }
    }
};

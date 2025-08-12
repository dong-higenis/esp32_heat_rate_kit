
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100.h"
#include "MAX30100_Registers.h"
#include "algorithm.h"
#include <PulseSensorPlayground.h>

#define I2C_SCL 22
#define I2C_SDA 21
#define BTN1   36
#define BTN2   39
#define BTN3   34
#define BTN4   35
#define HEART_RATE   32
#define LED1   25
#define LED2   26
#define BUZZER   5

#define SDA_1 16
#define SCL_1 17

#define SAMPLES BUFFER_SIZE  // 100

//  Test
#define TEST_LED_BLINK    0
#define TEST_BTN_INPUT    0
#define TEST_OLED_DISPLAY 0
#define TEST_HEART_RATE   1
#define TEST_BUZZER       0

//  Thread Sleep
#define TEST_INTERVAL 500

// oled
#define SCREEN_WIDTH  128  // OLED 가로 해상도
#define SCREEN_HEIGHT 64  // OLED 세로 해상도
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // 리셋 핀 -1 (없음)



#define USE_ARDUINO_INTERRUPTS true

const int OUTPUT_TYPE = SERIAL_PLOTTER;

const int PIN_INPUT = A4;
const int PIN_BLINK = 13;    // Pin 13 is the on-board LED
const int PIN_FADE = LED2;
const int THRESHOLD = 1500;   // Adjust this number to avoid noise when idle

PulseSensorPlayground pulseSensor;



#define NOTE_C7 2093
#define NOTE_E7 2637

int melody[] = {
  NOTE_C7, NOTE_E7, 0
};

int durations[] = {
  80, 120, 50
};


MAX30100 sensor;

static unsigned int un_brightness=0;
static unsigned int un_min=0x3FFFF;
static unsigned int un_max=0;
static unsigned int un_prev_data;

static unsigned int aun_ir_buffer[100]; //infrared LED sensor data
static unsigned int aun_red_buffer[100];  //red LED sensor data
static int n_ir_buffer_length=100; //data length
static int n_spo2;  //SPO2 value
static char ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
static int n_heart_rate; //heart rate value
static char  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
static unsigned char uch_dummy;
static uint16_t ir, red;

void setup(void)
{
  Serial.begin(115200);
  Wire1.begin(SDA_1, SCL_1);

  ledBlinkInit();
  btnInputInit();
  oledDisplayInit();
  heartRateInit();
  buzzerInit();
}

void loop(void)
{
  ledBlinkTask();
  btnInputTask();
  oledDisplayTask();
  heartRateTask();
  buzzerTask();
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void ledBlinkInit()
{
#if TEST_LED_BLINK
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
#endif
}

void ledBlinkTask()
{
#if TEST_LED_BLINK
  static bool led_state = false;
  static uint32_t prev_time = millis();
  if (millis() - prev_time >= TEST_INTERVAL)
  {
    prev_time = millis();

    led_state = !led_state;
    digitalWrite(LED1, led_state ? HIGH : LOW);
    digitalWrite(LED2, led_state ? HIGH : LOW);
  }
#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void btnInputInit()
{
#if TEST_BTN_INPUT
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);
#endif
}

void btnInputTask()
{
#if TEST_BTN_INPUT

  int btn_1 = 0;
  int btn_2 = 0;
  int btn_3 = 0;
  int btn_4 = 0;

  static uint32_t prev_time = millis();
  if (millis() - prev_time >= TEST_INTERVAL)
  {
    prev_time = millis();

    btn_1 = digitalRead(BTN1);
    btn_2 = digitalRead(BTN2);
    btn_3 = digitalRead(BTN3);
    btn_4 = digitalRead(BTN4);

    Serial.print("[");
    Serial.print(btn_1);
    Serial.print(btn_2);
    Serial.print(btn_3);
    Serial.print(btn_4);
    Serial.print("]");
    
    if (btn_1 == LOW) 
    {
      Serial.print("BTN1 Push,");
    } 
    else 
    {
      Serial.print("BTN1 Pull,");
    }

    if (btn_2 == LOW) 
    {
      Serial.print("BTN2 Push,");
    } 
    else 
    {
      Serial.print("BTN2 Pull,");
    }

    if (btn_3 == LOW) 
    {
      Serial.print("BTN3 Push,");
    } 
    else 
    {
      Serial.print("BTN3 Pull,");
    }

    if (btn_4 == LOW) 
    {
      Serial.print("BTN4 Push");
    } 
    else 
    {
      Serial.print("BTN4 Pull");
    }
    Serial.println(",");
  }
#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void oledDisplayInit()
{
#if TEST_OLED_DISPLAY
  // OLED 초기화
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 초기화 실패"));
    while (true);  // 멈춤
  }

  display.clearDisplay();
  display.setTextSize(1);              
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(1, 1);            
  display.println(F("SSD1315 Test!"));
  display.display();                  
#endif
}

void oledDisplayTask()
{
#if TEST_OLED_DISPLAY

#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void heartRateInit()
{
#if TEST_HEART_RATE

  pulseSensor.analogInput(PIN_INPUT);
  pulseSensor.blinkOnPulse(PIN_BLINK);
  pulseSensor.fadeOnPulse(PIN_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  if (!pulseSensor.begin()) 
  {
    for(;;) 
    {
      // Flash the led to show things didn't work.
      digitalWrite(PIN_BLINK, LOW);
      delay(50);
      digitalWrite(PIN_BLINK, HIGH);
      delay(50);
    }
  }

    Serial.print("Initializing MAX30100..");

    if (!sensor.begin()) {
        Serial.print("FAILED: ");

        uint8_t partId = sensor.getPartId();
        if (partId == 0xff) {
            Serial.println("I2C error");
        } else {
            Serial.print("wrong part ID 0x");
            Serial.print(partId, HEX);
            Serial.print(" (expected: 0x");
            Serial.println(EXPECTED_PART_ID, HEX);
        }
        // Stop here
        for(;;);
    } else {
        Serial.println("Success");
    }

    Serial.print("Enabling HR/SPO2 mode..");
    sensor.setMode(MAX30100_MODE_SPO2_HR);
    Serial.println("done.");

    Serial.print("Configuring LEDs biases to 50mA..");
    sensor.setLedsCurrent(MAX30100_LED_CURR_50MA, MAX30100_LED_CURR_50MA);
    Serial.println("done.");

    delay(1000);

    Serial.print("Lowering the current to 7.6mA..");
    sensor.setLedsCurrent(MAX30100_LED_CURR_7_6MA, MAX30100_LED_CURR_7_6MA);
    Serial.println("done.");

    delay(1000);

    Serial.print("Shutting down..");
    sensor.shutdown();
    Serial.println("done.");

    delay(1000);

    Serial.print("Resuming normal operation..");
    sensor.resume();
    delay(500);
    Serial.println("done.");

    uint32_t tsTempSampStart = millis();
    Serial.print("Sampling die temperature..");
    sensor.startTemperatureSampling();
    while(!sensor.isTemperatureReady()) {
        if (millis() - tsTempSampStart > 1000) {
            Serial.println("ERROR: timeout");
            // Stop here
            for(;;);
        }
    }

    float temperature = sensor.retrieveTemperature();
    Serial.print("done, temp=");
    Serial.print(temperature);
    Serial.println("C");

    if (temperature < 5) {
        Serial.println("WARNING: Temperature probe reported an odd value");
    } else {
        Serial.println("All test pass.");
    }

    Serial.println();

    sensor.resetFifo();

    for(int i=0;i<n_ir_buffer_length;i++)
    {
      sensor.update();
      sensor.getRawValues(&ir, &red);
      
      aun_ir_buffer[i] = ir;
      aun_red_buffer[i] = red;
    }
    un_prev_data=aun_red_buffer[n_ir_buffer_length - 1];
    //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
#endif
}

void heartRateTask()
{
#if TEST_HEART_RATE
  static uint32_t prev_time = millis();
  if (millis() - prev_time >= 20)
  {
    prev_time = millis();

    pulseSensor.outputSample();
    if (pulseSensor.sawStartOfBeat()) 
    {
      pulseSensor.outputBeat();
    }

    sensor.update();
    while (sensor.getRawValues(&ir, &red)) 
    {
        Serial.print("IR=");
        Serial.print(ir);
        Serial.print(" RED=");
        Serial.println(red);
    }

  }
#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void buzzerInit()
{
#if TEST_BUZZER
  pinMode(BUZZER, OUTPUT);
  // digitalWrite(BUZZER, HIGH);
  // delay(1000);
  // digitalWrite(BUZZER, LOW);
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) 
  {
    int note = melody[i];
    if (note == 0) 
    {
      noTone(BUZZER);
    } 
    else 
    {
      tone(BUZZER, note, durations[i]);
    }
    delay(durations[i] + 30);  // 짧은 텀
  }
  
  noTone(BUZZER);
#endif
}

void buzzerTask()
{
#if TEST_BUZZER
  static uint32_t prev_time = millis();
  if (millis() - prev_time >= TEST_INTERVAL)
  {
    prev_time = millis();

  }
#endif
}










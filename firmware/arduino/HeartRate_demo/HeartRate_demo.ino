// ================================

//            테스트 완료

// ================================

// ========== 예제 특징 ============

//  최종 코드입니다.

// ================================

// ======== 사용 라이브러리 =========
// MAX3010x : 1.1.2
// Adafruit_GFX : 1.12.1
// Adafruit_SSD1306 : 2.5.13
// =================================

// #include <Arduino.h>  // PlatformIO/VSCode 사용 시 권장

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

#define LED1_PIN 25
#define LED2_PIN 26

#define BTN1_PIN 36
#define BTN2_PIN 39
#define BTN3_PIN 34
#define BTN4_PIN 35

#define HEART_RATE_PIN 32
#define BUZZER_PIN 5

//  - OLED I2C 핀 (보드에 따라 변경 가능)

#define SCL_OLED_PIN 22
#define SDA_OLED_PIN 21

//  - MAX30102 센서 핀

#define I2C_SCL_PIN 17
#define I2C_SDA_PIN 16

// 예제 선택

// -사용 할 기능을 1로, 나머지는 0으로 둡니다.

#define USE_TEST_LED_BLINK 0
#define USE_TEST_BTN_INPUT 0
#define USE_TEST_OLED_DISPLAY 0
#define USE_TEST_HEART_RATE 0
#define USE_TEST_SPO_2 0
#define USE_TEST_BUZZER 1

// 각각의 예제에 대한 주기 제어

#define LED_INTERVAL_TIME_MS 500
#define BTN_INTERVAL_TIME_MS 500
#define OLED_INTERVAL_TIME_MS 200
#define BUZZER_INTERVAL_TIME_MS 500

// OLED 해상도 및 I2C통신 주소

#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64
#define OLED_I2C_ADDR 0x3C

// 그래프 크기 및 출력 시작위치

#define GRAPH_WIDTH 120
#define GRAPH_HEIGHT 32
#define GRAPH_START_X 4
#define GRAPH_START_Y 20

#define DISTANCE 90000 // 손가락과 센서사이의 거리

#define BPM_MAX 200 // BPM 최대치
#define BPM_MIN 20  // BPM 최소치

#define SPO_MIN 70 // 산소포화도 최소치

// 변수

// 주기 제어를 위한 변수
uint32_t led_prev_time;
uint32_t btn_prev_time;

// LED ON/OFF 상태 변수
bool led_state = false;

// 심박수 관련 변수

int32_t irValue;     // 적외선으로 검출한 센서 값 저장용 변수
int32_t beatForDiff; // 심박(BPM) 계산용 변수
int32_t lastBeat;    // 심박(BPM) 계산용 변수
int16_t RealBPM;     // 계산된 심박수
uint8_t beatAvg;     // 평균 심박수

// IR 값을 기준으로 그래프 스케일링

int32_t irBuffer[120];       // IR 센서값을 저장하는 그래프 버퍼 (Raw Data)
int32_t bufferIndex = 0;     // 버퍼에서 현재 쓰고 있는 위치
int32_t currentMin = 999999; // 현재 수집 중인 최소값
int32_t currentMax = 0;      // 현재 수집 중인 최대값
int32_t displayMin = 999999; // 화면 표시용 최소값 (스케일링용)
int32_t displayMax = 0;      // 화면 표시용 최대값 (스케일링용)

// ==== 산소 측정 =============================

// 센서 설정값
uint8_t ledBrightness = 60; // Options: 0=Off to 255=50mA
uint8_t sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, 32
uint8_t ledMode = 2;        // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
uint8_t sampleRate = 100;   // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
uint32_t pulseWidth = 411;  // Options: 69, 118, 215, 411
uint32_t adcRange = 4096;   // Options: 2048, 4096, 8192, 16384

uint32_t SPO2_irBuffer[100];  // 적외선 데이터 100개 저장용 배열
uint32_t SPO2_redBuffer[100]; // 빨간빛 데이터 100개 저장용 배열

// 계산 결과 저장 변수
int32_t bufferLength = 100; // 배열 크기 (100개)
int32_t spo2;               // 계산된 산소포화도 값
int8_t validSPO2;           // SPO2 값이 유효한지 여부 (1=유효, 0=무효)
int32_t heartRate;          // 계산된 심박수 값
int8_t validHeartRate;      // 심박수 값이 유효한지 여부

// ==== Buzzer ==============================
// ===== 부저 관련 상수 설명 =====
/*
 * 음표 주파수 정의:
 * NOTE_C7 = 2093Hz  // 높은 도 음
 * NOTE_E7 = 2637Hz  // 높은 미 음
 * 
 * melody[] 배열:
 * - 재생할 음표들의 주파수를 순서대로 저장
 * - 0은 쉼표(무음)를 의미
 * - 현재: 도-미-쉼표 순서로 재생
 * 
 * durations[] 배열:
 * - 각 음표의 재생 시간(밀리초)
 * - melody 배열과 순서가 일치해야 함
 * - 현재: 80ms-120ms-50ms
 * 
 * 부저 동작 원리:
 * - tone() 함수는 지정된 주파수로 사각파를 생성
 * - 주파수가 높을수록 높은 음, 낮을수록 낮은 음
 * - noTone()으로 소리를 완전히 중지
 */

#define NOTE_C7 2093
#define NOTE_E7 2637

int melody[] = {
    NOTE_C7, NOTE_E7, 0};

int durations[] = {
    80, 120, 50};

// ======= OLED 초기화 =============
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire1, -1);

// MAX3010x 인스턴스 생성
MAX30105 sensor;


// 함수 프로토 타입 선언

void ledBlinkInit();
void ledBlinkTask();
void btnInputInit();
void btnInputTask();
void oledDisplayInit();
void heartRateInit();
void heartRateTask();
void drawHeartRateGraph();
void SPO_Init();
void SPO_Task();
void Buzzer_Init();
void BuzzerTask();


void setup()
{

  Serial.begin(115200);
  Wire1.begin(SDA_OLED_PIN, SCL_OLED_PIN);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);

  ledBlinkInit();
  btnInputInit();
  oledDisplayInit();
  heartRateInit();
  SPO_Init();
  Buzzer_Init();
}

void loop()
{

  ledBlinkTask();
  btnInputTask();
  heartRateTask();
  SPO_Task();
  BuzzerTask();
}

// ============ Custom 함수 =================

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void ledBlinkInit()
{
#if USE_TEST_LED_BLINK

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

#endif
}

void ledBlinkTask()
{
#if USE_TEST_LED_BLINK

  if (millis() - led_prev_time >= LED_INTERVAL_TIME_MS) // 500ms마다 led1,2 토글
  {
    led_prev_time = millis(); // cpu 가동시간으로 부터 시간 측정

    led_state = !led_state;
    digitalWrite(LED1_PIN, led_state ? HIGH : LOW);
    digitalWrite(LED2_PIN, led_state ? HIGH : LOW);
  }

#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void btnInputInit()
{
#if USE_TEST_BTN_INPUT

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
  pinMode(BTN4_PIN, INPUT_PULLUP);

#endif
}

void btnInputTask()
{
#if USE_TEST_BTN_INPUT

  int btn_1 = 0;
  int btn_2 = 0;
  int btn_3 = 0;
  int btn_4 = 0;

  if (millis() - btn_prev_time >= BTN_INTERVAL_TIME_MS)
  {
    btn_prev_time = millis();

    btn_1 = digitalRead(BTN1_PIN);
    btn_2 = digitalRead(BTN2_PIN);
    btn_3 = digitalRead(BTN3_PIN);
    btn_4 = digitalRead(BTN4_PIN);

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
#if USE_TEST_OLED_DISPLAY

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR))
  {
    Serial.println(F("SSD1306 초기화 실패"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 1);
  display.println(F("OLED Test!"));
  display.display();

#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void heartRateInit()
{
#if USE_TEST_HEART_RATE
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR))
  {
    Serial.println(F("SSD1306 초기화 실패"));
    while (true);
  }

  // MAX30105 센서 초기화 및 연결 확인
  if (!sensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX3010x was not found. Please check wiring/power. ");
    while (1);
  }


  Serial.println("Place your index finger on the sensor with steady pressure.");
  // 센서 설정
  sensor.setup();
  sensor.setPulseAmplitudeRed(0x0A);
  sensor.setPulseAmplitudeGreen(0);

#endif
}

void heartRateTask()
{
#if USE_TEST_HEART_RATE

  irValue = sensor.getIR(); // 적외선 센서 값 읽기 (손가락 감지 및 혈류 변화 측정)

  irBuffer[bufferIndex] = irValue; // 값을 그래프용 버퍼에 넣기

  // 최대/최소값 업데이트 (자동 스케일링용)
  if (irValue > currentMax)
  {
    currentMax = irValue;
  }
  if (irValue < currentMin && irValue > 10000)
  {
    currentMin = irValue;
  }

  // 버퍼 인덱스를 다음으로 이동 (0~119 순환)
  bufferIndex = (bufferIndex + 1) % GRAPH_WIDTH;
  // 한 바퀴 돌면 스케일 업데이트
  if (bufferIndex == 0)
  {
    displayMin = currentMin;
    displayMax = currentMax;
    currentMin = 999999;
    currentMax = 0;
  }

  // ========== 심박 감지 및 BPM 계산 ==========
  // checkForBeat(): 라이브러리 내장 함수로 심박의 피크(최고점)를 감지

  if (checkForBeat(irValue) == true)
  {
    // 심박이 감지되었을 때 실행되는 코드

    // 이전 심박과 현재 심박 사이의 시간 간격 계산 (ms)
    beatForDiff = millis() - lastBeat;
    lastBeat = millis();

    // BPM 계산: 60초 ÷ (심박간격을 초단위로 변환)
    // 예) 심박간격이 1000ms(1초)면 → 60÷1 = 60BPM
    RealBPM = 60 / (beatForDiff / 1000.0);
  }

  static uint32_t prevDisp = 0;
  if (millis() - prevDisp >= OLED_INTERVAL_TIME_MS)
  { // 100ms마다 화면 업데이트
    prevDisp = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("BPM:");
    display.print((int)RealBPM);

    display.setCursor(0, 8);
    display.print("IR:");
    display.print(irValue / 1000);
    display.print("k");

    drawHeartRateGraph();

    display.setCursor(0, 56);
    if (irValue < DISTANCE)
    {
      display.print("NO FINGER");
    }
    else
    {
      display.print("Signal: OK");
    }

    display.display();
  }

  // 시리얼 출력
  Serial.print("IR=");
  Serial.print(irValue); // 적외선 센서 RAW 값 (손가락 접촉 확인용)
  Serial.print(", BPM=");
  Serial.print(RealBPM); // 현재 측정된 심박수

  // ========== 손가락 접촉 상태 확인 ==========
  // irValue가 100000보다 작으면 손가락이 제대로 접촉되지 않은 상태

  if (irValue < DISTANCE)
  {
    Serial.print(" No finger?");
  }
  Serial.println();

#endif
}

void drawHeartRateGraph()
{
  #if USE_TEST_HEART_RATE
  // 그래프 테두리
  display.drawRect(GRAPH_START_X - 1, GRAPH_START_Y - 1, GRAPH_WIDTH + 2, GRAPH_HEIGHT + 2, SSD1306_WHITE);

  // 스케일링 범위가 유효한 경우에만 그래프 그리기
  if (displayMax > displayMin && displayMax > 10000)
  {

    for (int i = 1; i < GRAPH_WIDTH; i++)
    {
      int currentIndex = (bufferIndex + i - 1) % GRAPH_WIDTH;
      int nextIndex = (bufferIndex + i) % GRAPH_WIDTH;

      // 자동 스케일링으로 Y 좌표 계산
      int y1 = map(irBuffer[currentIndex], displayMin, displayMax,
                   GRAPH_START_Y + GRAPH_HEIGHT - 1, GRAPH_START_Y);
      int y2 = map(irBuffer[nextIndex], displayMin, displayMax,
                   GRAPH_START_Y + GRAPH_HEIGHT - 1, GRAPH_START_Y);

      // 범위 제한
      y1 = constrain(y1, GRAPH_START_Y, GRAPH_START_Y + GRAPH_HEIGHT - 1);
      y2 = constrain(y2, GRAPH_START_Y, GRAPH_START_Y + GRAPH_HEIGHT - 1);

      // 선 그리기
      display.drawLine(GRAPH_START_X + i - 1, y1, GRAPH_START_X + i, y2, SSD1306_WHITE);
    }
  }
  #endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void SPO_Init(void)
{
#if USE_TEST_SPO_2

  // OLED 시작!
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR))
  {
    Serial.println(F("SSD1306 초기화 실패"));
    while (true);
  }

  // MAX30105 센서 초기화 및 연결 확인
  if (!sensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX3010x 센서를 찾을 수 없습니다. 연결을 확인 해 주십시오. ");
    while (1)
      ; // 무한루프로 프로그램 정지 (센서 연결 실패시)
  }

  Serial.println("안정적이게 손가락을 센서에 밀착해 주십시오.");

  // 조금 더 정교한 센서 설정
  sensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

#endif
}

void SPO_Task(void)
{
#if USE_TEST_SPO_2
  // 정적 변수 (함수 호출 간에 값이 유지됨)
  static uint32_t lastRead = 0;   // 마지막으로 OLED를 업데이트한 시간
  static uint8_t idx = 0;         // 배열에 데이터를 저장할 위치
  static bool collecting = false; // 데이터 수집 중인지 여부

  sensor.check(); // 센서에 새로운 데이터가 있는지 확인

  // 데이터 수집
  if (sensor.available()) // 새로운 데이터가 준비되었다면
  {
    SPO2_redBuffer[idx] = sensor.getRed(); // 빨간빛 값 저장
    SPO2_irBuffer[idx] = sensor.getIR();   // 적외선 값 저장
    sensor.nextSample();                   // 다음 샘플로 이동

    idx++;             // 다음 배열 위치로 이동
    collecting = true; // 데이터 수집 중임을 표시

    // 100개 모이면 산소 포화도 계산
    if (idx >= bufferLength) // 배열이 가득 찼다면
    {
      // SPO2와 심박수 계산 (라이브러리 함수 사용)
      maxim_heart_rate_and_oxygen_saturation(
          SPO2_irBuffer, bufferLength, SPO2_redBuffer,
          &spo2, &validSPO2, &heartRate, &validHeartRate);
      idx = 0; // 배열 인덱스 초기화 (다시 처음부터 채우기 시작)
    }
  }

  // 200ms마다 화면 업데이트
  if (millis() - lastRead > OLED_INTERVAL_TIME_MS)
  {
    lastRead = millis();

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);

    uint32_t ir = sensor.getIR();
    if (ir > DISTANCE && collecting)
    {
      display.print("SPO2: ");
      if (validSPO2 && spo2 > SPO_MIN)
      {
        display.print(spo2);
      }
      else
      {
        display.print("--");
      }
    }
    else
    {
      // 손가락이 감지되지 않았을때
      display.print("Put !!");
    }

    display.display();
  }
  if (validSPO2) // 유효한 값일때 계산된 산소포화도 시리얼 출력!
  {
    Serial.print("SPO2: ");
    Serial.print(spo2);
    Serial.println("%");
  }
#endif
}

//
//////////////////////////////////////////////////////////////////////////////////////////
//

void Buzzer_Init(void)
{
#if USE_TEST_BUZZER 
  // 부저를 제어하기 위해 BUZZER_PIN(5번 핀)을 출력으로 설정
  pinMode(BUZZER_PIN, OUTPUT);

  // 초기화 시 환영 멜로디 재생
  // 프로그램이 시작될 때 한 번만 멜로디를 재생합니다

  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    int note = melody[i]; // 현재 재생할 음표 가져오기

    if (note == 0) // 0은 쉼표(무음)를 의미
    {
      noTone(BUZZER_PIN); // 부저 소리 중지 (쉬기)
    }
    else // 실제 음표인 경우
    {
      // tone(핀번호, 주파수, 지속시간)
      // 지정된 주파수로 지정된 시간만큼 소리 재생
      tone(BUZZER_PIN, note, durations[i]);
    }
    // 음표 재생 후 짧은 여백 시간 추가
    // 각 음표 사이에 30ms의 간격을 둬서 음이 구분되도록 함
    delay(durations[i] + 30);
  }
  // 멜로디 재생 완료 후 부저 완전히 끄기
  noTone(BUZZER_PIN); // 모든 소리 중지
#endif
}

void BuzzerTask()
{
#if USE_TEST_BUZZER
  
  // ===== 주기적 실행을 위한 타이머 =====
  // static 변수는 함수가 끝나도 값이 유지됩니다
  static uint32_t prev_time = millis();

  // ===== 지정된 주기마다 실행 (현재 500ms) =====
  if (millis() - prev_time >= BUZZER_INTERVAL_TIME_MS)
  {
    prev_time = millis(); // 현재 시간을 기록

    
    // ===== 여기에 주기적으로 실행할 부저 코드 작성 =====
    // 현재는 비어있음 - 필요시 추가 기능 구현 가능
    // 예: 특정 조건에서 경고음 재생, 버튼 눌림 시 소리 등
    
    // 예시: 주기적으로 경고음 재생하고 싶다면
    // tone(BUZZER_PIN, 1000, 100);  // 1000Hz로 100ms 동안 소리
  }
#endif
}
/**
 * @brief OLED 그래프 예제
 *
 */

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

//  - OLED I2C 핀 (보드에 따라 변경 가능)

#define SCL_OLED_PIN 22
#define SDA_OLED_PIN 21

//  - MAX30102 센서 핀

#define I2C_SCL_PIN 17
#define I2C_SDA_PIN 16

// 주기 제어

#define OLED_INTERVAL_TIME_MS 200

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

// 변수

int32_t irValue; // 적외선으로 검출한 센서 값 저장용 변수
int32_t beatForDiff; // 심박(BPM) 계산용 변수
int32_t lastBeat; // 심박(BPM) 계산용 변수
int16_t RealBPM; // 계산된 심박수 
uint8_t beatAvg; // 평균 심박수

// IR 값을 기준으로 그래프 스케일링

int32_t irBuffer[120];       // IR 센서값을 저장하는 그래프 버퍼 (Raw Data)
int32_t bufferIndex = 0;     // 버퍼에서 현재 쓰고 있는 위치
int32_t currentMin = 999999; // 현재 수집 중인 최소값
int32_t currentMax = 0;      // 현재 수집 중인 최대값
int32_t displayMin = 999999; // 화면 표시용 최소값 (스케일링용)
int32_t displayMax = 0;      // 화면 표시용 최대값 (스케일링용)


// OLED 초기화
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire1, -1); // 하드웨어적 리셋이 필요없는경우 -1

// MAX3010x 라이브러리 인스턴스 생성
MAX30105 sensor;

void setup()
{

  Serial.begin(115200);
  Wire1.begin(SDA_OLED_PIN, SCL_OLED_PIN);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);

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

    // MAX30105 센서 초기화 및 연결 확인
  if (!sensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX3010x 센서를 찾을 수 없습니다. 연결을 확인 해 주십시오. ");
    while (1); // 무한루프로 프로그램 정지 (센서 연결 실패)
  }

  Serial.println("안정적이게 손가락을 센서에 밀착해 주십시오.");

  // 센서 설정
  sensor.setup(); // 센서를 기본 설정으로 초기화
  sensor.setPulseAmplitudeRed(0x0A); // 빨간 LED 밝기 설정 (0x0A = 낮은 밝기)
  sensor.setPulseAmplitudeGreen(0);  // 초록 LED 끄기 (심박 측정에는 빨강+적외선만 사용)

}



void loop()
{

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
  if (millis() - prevDisp >= 100)
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

}

void drawHeartRateGraph()
{
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
}
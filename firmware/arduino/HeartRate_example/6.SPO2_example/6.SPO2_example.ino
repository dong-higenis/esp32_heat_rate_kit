/**
 * @brief OLED에 산소포화도(SPO2) 값을 표시하는 예제
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
#include "heartRate.h"// 심박수 계산 라이브러리
#include "spo2_algorithm.h"// 산소포화도 계산 라이브러리

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

#define SPO_MIN 70 // 최소 산소포화도

// ==== 산소 측정 =============================

// 센서 설정값
uint8_t ledBrightness = 60; // Options: 0=Off to 255=50mA
uint8_t sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, 32
uint8_t ledMode = 2;        // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
uint8_t sampleRate = 100;   // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
uint32_t pulseWidth = 411;  // Options: 69, 118, 215, 411
uint32_t adcRange = 4096;   // Options: 2048, 4096, 8192, 16384

// 데이터 저장용 배열
uint32_t SPO2_irBuffer[100]; // 적외선 데이터 100개 저장용 배열
uint32_t SPO2_redBuffer[100]; // 빨간빛 데이터 100개 저장용 배열

// 계산 결과 저장 변수
int32_t bufferLength = 100; // 배열 크기 (100개)
int32_t spo2; // 계산된 산소포화도 값
int8_t validSPO2; // SPO2 값이 유효한지 여부 (1=유효, 0=무효)
int32_t heartRate; // 계산된 심박수 값
int8_t validHeartRate; // 심박수 값이 유효한지 여부

// ======= OLED 초기화 =============
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire1, -1);

// MAX3010x 인스턴스 생성
MAX30105 sensor;

void setup()
{

  Serial.begin(115200);
  Wire1.begin(SDA_OLED_PIN, SCL_OLED_PIN);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);

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
}

void loop()
{
  // 정적 변수 (함수 호출 간에 값이 유지됨)
  static uint32_t lastRead = 0; // 마지막으로 OLED를 업데이트한 시간
  static uint8_t idx = 0; // 배열에 데이터를 저장할 위치
  static bool collecting = false; // 데이터 수집 중인지 여부

  sensor.check(); // 센서에 새로운 데이터가 있는지 확인

  // 데이터 수집
  if (sensor.available()) // 새로운 데이터가 준비되었다면
  {
    SPO2_redBuffer[idx] = sensor.getRed(); // 빨간빛 값 저장
    SPO2_irBuffer[idx] = sensor.getIR(); // 적외선 값 저장
    sensor.nextSample(); // 다음 샘플로 이동

    idx++; // 다음 배열 위치로 이동
    collecting = true;     // 데이터 수집 중임을 표시

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
}

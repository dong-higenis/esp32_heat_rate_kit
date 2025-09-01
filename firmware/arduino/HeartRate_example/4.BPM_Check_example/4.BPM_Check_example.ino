/**
 * @brief 심박센서 측정 예제
 *
 */

// ======== 사용 라이브러리 =========
// MAX3010x : 1.1.2
// Adafruit_GFX : 1.12.1
// Adafruit_SSD1306 : 2.5.13
// =================================

// #include <Arduino.h>  // PlatformIO/VSCode 사용 시 권장

#include <Wire.h>      // I2C 통신용 라이브러리
#include "MAX30105.h"  // 센서용 라이브러리
#include "heartRate.h" // 센서용 라이브러리

//  - MAX30102 센서 핀

#define I2C_SCL_PIN 17
#define I2C_SDA_PIN 16

// 변수

int32_t irValue; // 적외선으로 검출한 센서 값 저장용 변수
int32_t beatForDiff; // 심박(BPM) 계산용 변수
int32_t lastBeat; // 심박(BPM) 계산용 변수
int16_t RealBPM; // 계산된 심박수 
uint8_t beatAvg; // 평균 심박수

int32_t distance = 100000; // 손가락과 센서사이의 거리 변수

// MAX3010x 라이브러리 인스턴스 생성
MAX30105 sensor;

void setup()
{

  Serial.begin(115200);                         // 115200의 baudrate로 시리얼 연결
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000); // 센서에 대한, I2C 초기화 및 시작 , I2C 속도는 400000이 고속 모드입니다.

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

  Serial.print("IR=");
  Serial.print(irValue); // 적외선 센서 RAW 값 (손가락 접촉 확인용)
  Serial.print(", BPM=");
  Serial.print(RealBPM); // 현재 측정된 심박수

  // ========== 손가락 접촉 상태 확인 ==========
  // irValue가 100000보다 작으면 손가락이 제대로 접촉되지 않은 상태

  if (irValue < distance)
  {
    Serial.print(" No finger?");
  } 
  Serial.println();
}

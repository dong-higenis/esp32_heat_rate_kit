/**
 * @brief OLED 예제
 *
 */

// ======== 사용 라이브러리 =========
// (이 예제는 oled만 사용하므로 아래 헤더는 생략 가능합니다)
// // MAX3010x : 1.1.2
// =================================

// #include <Arduino.h>  // PlatformIO/VSCode 사용 시 권장

#include <Wire.h> // I2C 통신을 위한 라이브러리
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 핀 번호

//  - OLED I2C 핀 (보드에 따라 변경 가능)

#define SCL_OLED_PIN 22
#define SDA_OLED_PIN 21

// 주기 제어

#define OLED_INTERVAL_TIME_MS 200

// OLED 해상도 및 I2C통신 주소

#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64
#define OLED_I2C_ADDR 0x3C

// OLED 초기화
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1); // 하드웨어적 리셋이 필요없는경우 -1

void setup()
{

  // 시리얼 초기화 및 시작
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR))
  {
    Serial.println(F("SSD1306 초기화 실패"));
    while (true);
  }
// OLED 화면 초기 설정

  display.clearDisplay(); // 화면을 완전히 지우기 (검은 화면)
  display.setTextSize(1); // 텍스트 크기 설정 (1=8x8픽셀, 2=16x16픽셀...)
  display.setTextColor(SSD1306_WHITE); // 텍스트 색상 (WHITE=밝음, BLACK=어두움)
  display.setCursor(1, 1); // 텍스트 시작 위치 (x=1픽셀, y=1픽셀)
  display.println(F("OLED Test!")); // 화면 버퍼에 텍스트 쓰기
  display.display(); // 실제 화면에 버퍼 내용을 출력 (이 명령어가 있어야 화면에 보임!)
}

void loop()
{
  // 현재는 비어있음 - 한번만 텍스트를 출력하고 끝
  // 여기에 추가 기능 구현 가능 (버튼, LED, 센서 등, 원하는 기능 추가!)
}

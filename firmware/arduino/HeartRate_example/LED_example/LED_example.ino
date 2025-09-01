/**
 * @brief LED 예제
 */

// ======== 사용 라이브러리 =========
// (이 예제는 LED 토글만 사용하므로 아래 헤더는 생략 가능합니다)
// // MAX3010x : 1.1.2
// // Adafruit_GFX : 1.12.1
// // Adafruit_SSD1306 : 2.5.13
// =================================

// #include <Arduino.h>  // PlatformIO/VSCode 사용 시 권장

// 핀 번호

#define LED1_PIN 25
#define LED2_PIN 26

// 주기 제어

#define LED_INTERVAL_TIME_MS 500 // (500ms)

// 주기 제어를 위한 변수
uint32_t prev_time;

// LED ON/OFF 상태 변수
bool led_state = false;

void setup()
{
  // 시리얼 초기화 및 시작
  Serial.begin(115200);

  // LED1, LED2 핀을 출력모드로 설정합니다.
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop()
{
  

  if (millis() - prev_time >= LED_INTERVAL_TIME_MS) // 500ms마다 led1,2 토글
  {
    // cpu 가동시간으로 부터 시간 측정
    prev_time = millis();

    // False -> True
    led_state = !led_state;
    // led_state = True 면 HIGH, False 면 LOW 로서
    // LED가 주기적으로 깜빡거립니다.
    digitalWrite(LED1_PIN, led_state ? HIGH : LOW);
    digitalWrite(LED2_PIN, led_state ? HIGH : LOW);
  }
}

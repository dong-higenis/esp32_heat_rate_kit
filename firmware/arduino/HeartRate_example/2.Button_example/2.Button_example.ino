/**
 * @brief 버튼 예제
 * 
 */
// ======== 사용 라이브러리 =========
// (이 예제는 LED 토글만 사용하므로 아래 헤더는 생략 가능합니다)
// // MAX3010x : 1.1.2
// // Adafruit_GFX : 1.12.1
// // Adafruit_SSD1306 : 2.5.13
// =================================

// #include <Arduino.h>  // PlatformIO/VSCode 사용 시 권장

// 핀 번호

#define BTN1_PIN 36
#define BTN2_PIN 39
#define BTN3_PIN 34
#define BTN4_PIN 35

// 주기 제어

#define BTN_INTERVAL_TIME_MS 500 // (500ms)

// 주기 제어를 위한 변수

uint32_t prev_time;

// 버튼 눌림 상태 변수

int btn_1_state = 0;
int btn_2_state = 0;
int btn_3_state = 0;
int btn_4_state = 0;

void setup()
{
  // 시리얼 초기화 및 시작
  Serial.begin(115200);
  
  // 버튼들을 입력모드로 설정합니다.
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT);
  pinMode(BTN3_PIN, INPUT);
  pinMode(BTN4_PIN, INPUT);

}

void loop()
{
  

  if (millis() - prev_time >= BTN_INTERVAL_TIME_MS) // 500ms마다 버튼상태 체크
  {
    // cpu 가동시간으로 부터 시간 측정
    prev_time = millis();

    // 버튼이 눌렸는지 여부를 입력 받습니다.
    btn_1_state = digitalRead(BTN1_PIN);
    btn_2_state = digitalRead(BTN2_PIN);
    btn_3_state = digitalRead(BTN3_PIN);
    btn_4_state = digitalRead(BTN4_PIN);

    // 회로상, 버튼은 풀업 되어있으므로, LOW가 눌림 입니다.
    // 버튼이 눌리면 해당 버튼에 대한 상태메시지가 시리얼 모니터에 출력 됩니다.
    if (btn_1_state == LOW)
    {
      Serial.print("BTN1 Push,");
    }
    else
    {
      Serial.print("BTN1 Pull,");
    }

    if (btn_2_state == LOW)
    {
      Serial.print("BTN2 Push,");
    }
    else
    {
      Serial.print("BTN2 Pull,");
    }

    if (btn_3_state == LOW)
    {
      Serial.print("BTN3 Push,");
    }
    else
    {
      Serial.print("BTN3 Pull,");
    }

    if (btn_4_state == LOW)
    {
      Serial.print("BTN4 Push");
    }
    else
    {
      Serial.print("BTN4 Pull");
    }
    Serial.println(",");
  }

}
  

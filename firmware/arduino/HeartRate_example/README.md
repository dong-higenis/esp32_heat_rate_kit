# Heart_Rate_Sensor
ESP32를 사용한 심박센서 예제코드입니다.

예제 1) <LED_example.ino>
    500[ms] 마다 LED 1, LED 2를 토글하는 예제입니다.

예제 2) <Button_example.ino>
    500[ms] 마다 버튼의 눌림 여부를 확인하여,
    시리얼 모니터에 출력하는 예제입니다.

예제 3) <OLED_example.ino>
    I2C 통신을 이용하여, OLED 화면에 간단한 텍스트를 출력하는 예제입니다.

예제 4) <BPM_Check_example.ino>
    MAX30102 센서를 이용하여, BPM(심박수)를 측정한 후, 시리얼 모니터에 실시간으로 출력하는 예제입니다.

예제 5) <BPM_OLED_example.ino>
    예제 4의 결과를 그래프로 시각화 하여, OLED 화면에 출력하는 예제입니다.

예제 6) <SPO2_example.ino>
    산소 포화도 측정을 진행하고, 결과값을 OLED에 출력하는 예제입니다.

총합본 ) <HeartRate_demo.ino>
    예제 1 ~ 6 을 통합한 합본입니다. 
    
        #define USE_TEST_LED_BLINK 0
        #define USE_TEST_BTN_INPUT 0
        #define USE_TEST_OLED_DISPLAY 0
        #define USE_TEST_HEART_RATE 0
        #define USE_TEST_SPO_2 0
        #define USE_TEST_BUZZER 1
    
    원하는 예제 테스트를 위해 테스트가 필요한 기능을 0 -> 1 로 바꿔서 진행하시면 됩니다.
    
    추가로 부저 기능 테스트도 추가 하였습니다.


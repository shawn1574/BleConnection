
/*로드셀 설정*/
#include "HX711.h"
#define DOUT  34 //
#define CLK  35 // 
HX711 scale(DOUT, CLK);
float calibration_factor = 7280; //7050 worked for my 440lb max scale setup
float value = 0;
/*로드셀 설정*/

/*모터 설정*/
#define m_start_p    36 // START/STOP  LOW START HIGH STOP
#define m_run_p      37 // RUN/BRAKE   LOW RUN HIGH BRAKE
#define m_l_dir_p    38 // 왼쪽 모터 방향
#define m_r_dir_p    39 // 오른쪽 모터 방향
#define m_speed_p    7 // 모터 스피드
#define m_hall_a_p   41 // 홀센서 A
#define m_hall_b_p   42 // 홀센서 B
#define m_hall_c_p   43 // 홀센서 C

int speed_out = 0;
int speed_out_Temp = 0;
/*모터 설정*/
/*fsr 설정*/
#define F_fsr_p A8
#define B_fsr_p A9
int F_fsr = 0;
int B_fsr = 0;
/*fsr 설정*/

/*적외선 설정 GP2Y0A02*/ //한쪽만 사용
#define IRsensor1 A1
float sensorValue1, inches1, cm1;    //Must be of type float for pow()
const int numReadings1 = 10;       // 평균을 취할 데이터 갯수
long readings1[numReadings1];       // 신호값저장할 배열 지정
int readIndex1 = 0;                // 현재 신호값 index 변수 선언
long total1 = 0;                   // 합계 변수
long average1 = 0;                 // 평균값 저장 변수

#define IRsensor2 A2   // 사용안함
float sensorValue2, inches2, cm2;    //Must be of type float for pow()
const int numReadings2 = 10;       // 평균을 취할 데이터 갯수
long readings2[numReadings2];       // 신호값저장할 배열 지정
int readIndex2 = 0;                // 현재 신호값 index 변수 선언
long total2 = 0;                   // 합계 변수
long average2 = 0;
/*적외선 설정*/

/*시스템*/
int state = 0;
const int run_F = 1;
const int run_B = 2;
const int stop  = 3;
/*시스템*/

/*BT*/
int buffPos;
String inputString;
int mode_bt = 0; //0:하드웨어 1:BT
/*BT*/

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Load_cell_init();
  GP2Y0A02_init();
  mot_init();
  Load_cell_weight();
  analogWrite(m_speed_p, 0);
}


void loop() {
  serial_bt();
  Serial.print(mode_bt); Serial.print("\t");
  GP2Y0A02_1();
  if (mode_bt == 0) {

    fsr();
    Load_cell_weight();

    switch (state) {
      case run_F : // 앞쪽
        digitalWrite(m_run_p, LOW);
        digitalWrite(m_l_dir_p, LOW);
        digitalWrite(m_r_dir_p, HIGH);
        Serial.print("\t앞\t");
        if (speed_out > 20) {
          speed_out = 20;
          Serial.print("\눞음\t");
        } else {
          Serial.print("\작음\t");
        }
        Serial.print("\tspeed_out\t");
        Serial.print(speed_out);
        Serial.print("\t");

        speed_out = speed_out + map(value, 0, 100, 0, 5); // 로드셀 무게에따라 속도 추가
        if (cm1  <  50 ) {
          Serial.print("\위험\t");
          analogWrite(m_speed_p, 0);
          digitalWrite(m_run_p, HIGH);
        } else {
          Serial.print("\정상\t");
          digitalWrite(m_run_p, LOW);
          analogWrite(m_speed_p, speed_out);
        }

        break;
      case run_B : //뒷쪽
        digitalWrite(m_run_p, LOW);
        digitalWrite(m_l_dir_p, HIGH);
        digitalWrite(m_r_dir_p, LOW);
        Serial.print("\뒤\t");

        if (speed_out > 20) {
          speed_out = 20;
        }
        speed_out = speed_out + map(value, 0, 100, 0, 15); // 로드셀 무게에따라 속도 추가

        analogWrite(m_speed_p, speed_out);


        break;

      case stop : //정지

        if (speed_out <= 2 ) {
          digitalWrite(m_run_p, HIGH);
          Serial.print("\정지\t");
          speed_out = 0;
          analogWrite(m_speed_p, speed_out);
        } else if (speed_out > speed_out_Temp - 2) {
          speed_out = speed_out - 2; //서서히 감속
          Serial.print("\감속\t");
          analogWrite(m_speed_p, speed_out);
        }

        delay(5);
        break;
    }

    speed_out_Temp = speed_out;
    Serial.print("speed_out ");
    Serial.print( speed_out); Serial.print("  speed_out_Temp ");
    Serial.print( speed_out_Temp);
    Serial.print("\t");

  } else if (mode_bt == 1) {

  }

}

void Load_cell_init() {
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  scale.tare(); //Reset the scale to 0
}

void Load_cell_weight() {
  value = scale.get_units() * 0.453592;
  if (value < 0) {
    value = 0;
  }
  Serial.print("Reading: ");
  Serial.print(value, 2);
  Serial.print(" KG\t"); // Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person

}


void mot_init() {
  pinMode(m_start_p, OUTPUT);
  pinMode(m_run_p, OUTPUT);
  pinMode(m_l_dir_p, OUTPUT);
  pinMode(m_r_dir_p, OUTPUT);

  pinMode(m_hall_a_p, INPUT_PULLUP);
  pinMode(m_hall_b_p, INPUT_PULLUP);
  pinMode(m_hall_c_p, INPUT_PULLUP);

  digitalWrite(m_start_p, LOW);
  digitalWrite(m_run_p, LOW);

}

void GP2Y0A02_init() {
  for (int i = 0; i < numReadings1; i++) {
    readings1[i] = 0;
  }
}
void GP2Y0A02_1() {
  sensorValue1 = analogRead(IRsensor1);
  //inches = 4192.936 * pow(sensorValue,-0.935) - 3.937;
  cm1 = 10650.08 * pow(sensorValue1, -0.935) - 10;
  
  Serial.print("Distance(cm) = ");
  Serial.print(cm1);
  Serial.print("\t");
}
void GP2Y0A02_2() {  // 사용 안함
  sensorValue2 = analogRead(IRsensor2);
  //inches = 4192.936 * pow(sensorValue,-0.935) - 3.937;
  cm2 = 10650.08 * pow(sensorValue2, -0.935) - 10;
  total2 = total2 - readings2[readIndex2];              // readIndex가 가르키는 값을 제외
  readings2[readIndex2] = cm2;                         // 센서값을 읽어 배열에 저장
  total2 = total2 + readings2[readIndex2];              // 현재값을 Total에 저장
  readIndex2 = readIndex2 + 1;                        // 다음 배열에 저장하기 위해 readindex를 증가
  if (readIndex2 >= numReadings2) {                   // readIndex값이 10을 넘으면 리셋
    readIndex2 = 0;
  }
  average2 = total2 / numReadings2;                    // 평균치 계산
  delay(100);
  Serial.print("Distance(cm) = ");
  Serial.println(average2);
}

void fsr() { // 방향 결정 함수
  int F_data = analogRead(F_fsr_p);
  int B_data = analogRead(B_fsr_p);
  int dir = F_data - B_data;
  Serial.print(F_data);
  Serial.print("\t");
  Serial.print(B_data);
  Serial.print("\t");
  Serial.println(dir);
  /* fsr 세기 측정 부분 */

  if (dir > 50) {
    state = run_F;
    speed_out++;
  } else if (dir < -50) {
    state = run_B;
    speed_out++;
  } else {
    state = stop;
  }
}

void serial_bt() { // 블루투스 통신 함수
  String send_data = (String)value;
  send_data += "Kg";
  Serial1.println(value); // 무게 전송

  if (Serial1.available() > 0) { // 받는 데이터 함수
    buffPos++; // buffPos + 1 과 같다.
    char data = (char)Serial1.read(); //읽어들인 문자(한글자)를 data에 저장

    inputString += data; //data에 저장된 값을 inputString에 합쳐라
    Serial.print("data = ");
    Serial.println(data);
    if (data == '\n') { //개행문자(줄바꿈)가 들어오면 실행해라
      Serial.print("inputString = ");
      Serial.print(inputString);
      // 데이터 파싱 동작 부분 작성

      if (inputString[0] == 'T') {
        inputString.replace("\n", "");
        inputString.replace("\r", "");
        Serial.println(inputString[0]);
        if (inputString[1] == '0') {
          mode_bt = 0; // 하드웨어 모드
          Serial.println("hw");
        } else if (inputString[1] == '1') {
          mode_bt = 1; // BT 모드
          Serial.println("bt");
        }

      }
      if (inputString[0] == 'F') {
        inputString.replace("\n", "");
        inputString.replace("\r", "");
        inputString.replace("F", "");
        int bt_speed = inputString.toInt();
        digitalWrite(m_run_p, LOW);
        digitalWrite(m_l_dir_p, LOW);
        digitalWrite(m_r_dir_p, HIGH);
        bt_speed = constrain(bt_speed, 0, 255); // 어플에서 속도 제한 풀려있음
        analogWrite(m_speed_p, bt_speed);
      } else if (inputString[0] == 'B') {
        inputString.replace("\n", "");
        inputString.replace("\r", "");
        inputString.replace("B", "");
        int bt_speed = inputString.toInt();
        bt_speed = constrain(bt_speed, 0, 255); // 어플에서 속도 제한 풀려있음
        digitalWrite(m_run_p, LOW);
        digitalWrite(m_l_dir_p, HIGH);
        digitalWrite(m_r_dir_p, LOW);
        analogWrite(m_speed_p, bt_speed);
      } else if (inputString[0] == 'E') { // 긴급 정지
        inputString.replace("\n", "");
        inputString.replace("\r", "");
        inputString.replace("B", "");
        int bt_speed = inputString.toInt();
        bt_speed = constrain(bt_speed, 0, 255); // 어플에서 속도 제한 풀려있음
        digitalWrite(m_run_p, LOW);
        digitalWrite(m_l_dir_p, HIGH);
        digitalWrite(m_r_dir_p, LOW);
        analogWrite(m_speed_p, bt_speed);
      } else if (inputString[0] == 'E') { // 긴급 정지
        inputString.replace("\n", "");
        inputString.replace("\r", "");
        inputString.replace("E", "");
        digitalWrite(m_run_p, HIGH);
        analogWrite(m_speed_p, 0);
      } else if (inputString[0] == 'Z') {//정지
        while (1) {
          if (speed_out <= 2 ) {
            digitalWrite(m_run_p, HIGH);
            Serial.print("\정지\t");
            speed_out = 0;
            analogWrite(m_speed_p, speed_out);
            break;
          } else if (speed_out > speed_out_Temp - 2) {
            speed_out = speed_out - 2; //서서히 감속
            Serial.print("\감속\t");
            analogWrite(m_speed_p, speed_out);

          }
        }

      }

      buffPos = 0; // buffPos 초기화
      inputString = ""; //inputString 초기화

      Serial2.flush();
    }
  }
}


/*Read reference voltage*/
long readVref()  // 내부 전압 체크 함수 필요시 넣어서 사용
{
  long result;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_AT90USB1286__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRB &= ~_BV(MUX5);   // Without this the function always returns -1 on the ATmega2560 http://openenergymonitor.org/emon/node/2253#comment-11432
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#endif
#if defined(__AVR__)
  delay(2);                                        // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);                             // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result;  //1100mV*1024 ADC steps http://openenergymonitor.org/emon/node/1186
  return result;
#elif defined(__arm__)
  return (3300);                                  //Arduino Due
#else
  return (3300);                                  //Guess that other un-supported architectures will be running a 3.3V!
#endif
}

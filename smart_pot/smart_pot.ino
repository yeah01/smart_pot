#include <Wire.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>


SoftwareSerial BTSerial(2, 3); //RX, TX
LiquidCrystal_I2C lcd(0x27,16,2);
OneWire ds(4);


volatile int soil;
volatile int light;
volatile int water;
volatile int val;
volatile int temp;

// 전역 변수 선언 ///////////////////////////////
int CdsValue = 0;  //조도센서 값 저장 변수
int SoilWaterValue = 0; //수분센서 값 저장 변수
int SwValue = 0;   //스위치 상태 저장 변수
int WaterLevelValue = 0; //수위센서 값 저장 변수



void setup() {
  Serial.begin(9600);          // 시리얼 통신 시작, 통신속도 설정
  BTSerial.begin(9600);
  
  pinMode(5, OUTPUT);    // 펌프모터 핀모드 설정
  pinMode(9, OUTPUT);      // RGB_LED RED 핀모드 설정
  pinMode(10, OUTPUT);    // RGB_LED GREEN 핀모드 설정
  pinMode(11, OUTPUT);     // RGB_LED BLUE 핀모드 설정
  pinMode(12, INPUT_PULLUP); // 스위치 풀업저항 사용
  pinMode(A0, INPUT);     //토양수분센서
  pinMode(A1, INPUT);    //조도센서
  pinMode(A2, INPUT);    //수위센서
}

void loop() {

  SoilWaterValue = map(analogRead(A0), 0, 1023, 0,100 );
  SoilWaterValue = SoilWaterValue;
  CdsValue = analogRead(A1);
  SwValue = digitalRead(12);
  WaterLevelValue = analogRead(A2);

  byte i;
  byte present = 0;
  byte data[12]; //data 저장 공간 설정
  byte addr[8]; //addr 저장 공간 설정 
  float Temp; //Temp 변수 설정

  if (!ds.search(addr)) {
    ds.reset_search();
    return;
  }
  //no more sensors on chain, reset search

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); //start conversion, with parasite power on at the end
  delay(1000);

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for (i = 0; i < 9; i++) { 
    data[i] = ds.read();
  }

  Temp=(data[1]<<8)+data[0];
  Temp=Temp/16;


  Serial.print("C=");
  Serial.print(Temp);
  Serial.println(" ");
  //섭씨 온도 출력

  //시리얼 모니터에 출력하기
  Serial.print("SoilWaterValue : ");
  Serial.println(SoilWaterValue);
  Serial.print("CdsValue : ");
  Serial.println(CdsValue);
  Serial.print("WaterLevelValue : ");
  Serial.println(WaterLevelValue);
  delay(100);

   //LCD 창에 토양의 수분량을 백분율로 출력
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Soil Water: ");
  lcd.setCursor(12,0);
  lcd.print(SoilWaterValue);
  lcd.setCursor(15,0);
  lcd.print("%");
  
   //LCD CDS 출력 
  lcd.setCursor(0,1);
  lcd.print("CDS : ");
  lcd.setCursor(6,1);
  lcd.print(CdsValue);

// 물통에 물이 충분한지 확인
  if ( WaterLevelValue > 200 ) //물통에 물이 충분할 때
  {
    // 식물 성장에 도움을 주는 LED켜기
     if ( CdsValue > 700 )  //주변 환경에 맞게 센서 값 조정하기
     {
       analogWrite(9,255);//어두우면 Red
       analogWrite(10,0);
       analogWrite(11,0);
     }
     else
     {
      analogWrite(9,0);//밝으면 Blue
      analogWrite(10,0);
      analogWrite(11,255);    
     }
 
     // 수분이 부족하면 자동으로 물 공급    
     if ( SoilWaterValue < 25 )
      {
        analogWrite(5,255); //모터의 회전속도 조절 (0-255까지 조절 가능)
        delay ( 2000 ); //2초간 물주기
        analogWrite(5,0); //모터 회전 0
        delay ( 10000 ); //물주고 10초 딜레이 후 다시 수분 측정
      }
  }
  else  // 물통에 물이 부족하면 빨간색 LED 깜빡이기
  {
     analogWrite(9,255);
     analogWrite(10,0);
     analogWrite(11,0);
     delay ( 200 );
     analogWrite(9,0);
     analogWrite(10,0);
     analogWrite(11,0);
     delay ( 100 );
  }
  
   if (BTSerial.available() > 0) 
  {
    val = BTSerial.read();
  }
  
  switch (val) {
    case 'a': //조도센서 값 확인
      light = analogRead(A1);
      BTSerial.print("a:");
      BTSerial.println(light);
      delay(100);
      break;
    case 'b'://토양수분센서 값 확인
      soil = analogRead(A0);
      BTSerial.print("b:");
      BTSerial.println(soil);
      delay(100);
      break;
    case 'c'://수위센서 값 확인
      water = analogRead(A2);
      BTSerial.print("c:");
      BTSerial.println(water);
      delay(100);
      break;
    case 'd'://온도센서 값 확인
      temp = digitalRead(Temp);
      BTSerial.print("f:");
      BTSerial.println(Temp);
      delay(100);
      break;
    case 'e'://물공급 시작
      analogWrite(5,255);
      break;
    case 'f'://물공급 종료
      analogWrite(5,0);
      break;

  }
}

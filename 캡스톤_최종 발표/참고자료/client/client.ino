#include <SoftwareSerial.h>
#include <stdio.h>

//시리얼 통신 변수
SoftwareSerial lcdSerial(2, 3); //RX, TX

//키패드 변수
String keys="123A456B789C*0#D";
int key;
boolean key_lockout=false;

//조도값 변수
unsigned int cds=0, cdsTmp=0;
byte* pByte;

//토양센서 변수
unsigned int humidity = 0, humidityTmp=0;

//LCD 변수
int x, y;
byte ch;
char str[6];
int growLevel = 1, growSpeed = 1; //level, speed 초기값
int option = 1; // level로 초기값

//플렉시블 LED 바 변수 : arrLed[growLevel][growSpeed]
int cdsLevel=-1;
int arrLed[6][6] = {
  {0,0,0,0,0,1},{0,0,0,0,1,1},{0,0,0,1,1,1},{0,0,1,1,1,1},{0,1,1,1,1,1},{1,1,1,1,1,1}
};

//초기화 플래그
boolean isStart=true;

//키패드 함수
int getKeypad();

//LCD 함수
void cls();

void locate(int x, int y);
void lcdPrint(char* str);

void invisibleCsr();
void visibleCsr();
void offBlinkCsr();
void onBlinkCsr();

void offLight();
void onLight();

void backspace();
void enter();

void displayInit();
void printHumidity(int humidity);
void printCds(int cds);
void printValue();
void printCtg();

//LED(릴레이) 함수
void turnOnLed(int cdsLevel);
void turnOffLed();

void waterPump(int sec);

//프로그램 초기화
void setup(){
  //디지털 핀
  pinMode(4, OUTPUT); //4~9 : LED(릴레이)
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT); //10 : 수중펌프(릴레이)

  turnOffLed();
  digitalWrite(10, HIGH);

  //아날로그 핀
  pinMode(A1,INPUT); //조도센서
  pinMode(A2,INPUT); //토양센서
  
  lcdSerial.begin(9600);
  
  Serial.begin(9600);
  delay(200);
  
}

//프로그램 시작
void loop(){

  //초기화
  if(isStart){
    offLight();
    delay(1000);
    onLight();
    invisibleCsr();
    displayInit();

    isStart=false;

    locate(0,0);
  }

  //keyPad -> Ar
  key=getKeypad();
  if(key!=-1){
    //Ar -> Rasp
    Serial.write(0xB3);
    Serial.write((byte)keys[key]);
  }

  //Rasp -> Ar
  if(Serial.available()){
    ch = (byte)Serial.read();
    delay(1);

    //debug//
    if((char)ch=='0'){
      waterPump(3);
    }
    else if((char)ch>='1' && (char)ch<='9'){
      turnOnLed(ch-'0'-1);
      waterPump((char)ch-'0');
    }
    /////////

    //water
    if(ch==0xc1){
      waterPump((int)Serial.read());
    }
    else if(ch==0xc2){
      cdsLevel = (int)Serial.read();
      delay(1);
      
      if(cdsLevel == 0){
        turnOffLed();
      }
      else{
        turnOnLed(cdsLevel-1);
      }
     
    }
    //Ar -> LCD
    else if(ch==0xc3){
      ch = (byte)Serial.read();
      delay(1);
      sprintf(str, "%d", ch);
      option = atoi(str);

      ch = (byte)Serial.read();
      delay(1);
      
      if(ch == 0x04){
          if(option==1){
            growLevel--;
            if(growLevel<1){growLevel=1;}
          }
          else{
            growSpeed--;
            if(growSpeed<1){growSpeed=1;}
          }
          printValue();
      }
      else if(ch == 0x05){
          if(option==1){
            growLevel++;
            if(growLevel>3){growLevel=3;}
          }
          else{
            growSpeed++;
            if(growSpeed>2){growSpeed=2;}
          }
          printValue();
      }
      else if(ch == 0x03){
        printCtg();
        printValue();
      }
    }
  }

  //센싱

  //cds -> Ar
  cdsTmp = 1000-analogRead(A1);
  cdsTmp = (cdsTmp/10)*10;
  if(cds != cdsTmp){
    cds = cdsTmp;
    //Ar -> LCD
    printCds(cds);
    //Ar -> Rasp
    Serial.write(0xB2);
    pByte = (byte*)&cds;
    Serial.write(pByte[0]);
    Serial.write(pByte[1]);
    delay(1);
  }

  //토양센서 -> Ar
  humidityTmp = 1000-analogRead(A2);
  humidityTmp = constrain(humidityTmp,0,1023);
  humidityTmp /= 8;
  if(humidity != humidityTmp){
    humidity = humidityTmp;
    //Ar -> LCD
    printHumidity(humidity);
    //Ar -> Rasp
    Serial.write(0xB1);
    pByte = (byte*)&humidity;
    Serial.write(pByte[0]);
    Serial.write(pByte[1]);
    delay(1);
  }
  
}
 
int getKeypad(){
  int ret=-1;
  boolean reset_lockout=false;
  
  if(analogRead(A0)<184){
    key_lockout=false;
  }
  else if(!key_lockout){
    delay(20);
    ret=15-(log((analogRead(A0)-183.9)/58.24)/0.1623)+0.5;
    key_lockout=true;
  }
  
  return ret;
}

void cls(){
  lcdSerial.write(0xa3);
  lcdSerial.write(0x1);
}

void locate(int y, int x){
  lcdSerial.write(0xa1);
  lcdSerial.write(x);
  lcdSerial.write(y);
}

void lcdPrint(char* str){
  lcdSerial.write(0xa2);
  lcdSerial.write(str);
  lcdSerial.write((byte)0x00);
}

void invisibleCsr(){
  lcdSerial.write(0xa3);
  lcdSerial.write(0x0c);
}

void visibleCsr(){
  lcdSerial.write(0xa3);
  lcdSerial.write(0x0e);
}

void offBlinkCsr(){
  lcdSerial.write(0xa3);
  lcdSerial.write(0x0a);
}

void onBlinkCsr(){
  lcdSerial.write(0xa3);
  lcdSerial.write(0x0b);
}

void offLight(){
  lcdSerial.write(0xa5);
  lcdSerial.write(0x2);
}

void onLight(){
  lcdSerial.write(0xa5);
  lcdSerial.write(0x01);
}

void backspace(){
  lcdSerial.write(0xa6);
  lcdSerial.write(0x08);
}

void enter(){
  lcdSerial.write(0xa6);
  lcdSerial.write(0x0d);
}

void displayInit(){
  cls();
  locate(0, 0);
  lcdPrint("Humidity :");
  enter();
  lcdPrint("Bright   :");
  enter();
  lcdPrint("< Bright Level >");
  enter();
  lcdPrint("(*)<-  01  ->(#)");
}

void printHumidity(int humidity){
  locate(0, 11);
  sprintf(str, "%02d", humidity);
  lcdPrint(str);
}

void printCds(int cds){
  locate(1, 11);
  sprintf(str, "%02d", cds);
  lcdPrint(str);
}

void printCtg(){
  locate(2, 9);
  if(option==1){
    lcdPrint("Level");
  }
  else if(option==2){
    lcdPrint("Speed");
  }
}

void printValue(){
  locate(3, 7);

  if(option==1){
    sprintf(str, "%02d", growLevel);
  }
  else if(option==2){
    sprintf(str, "%02d", growSpeed);
  }
  lcdPrint(str);
}

void turnOnLed(int cdsLevel){
  digitalWrite(4, !arrLed[cdsLevel][5]);
  digitalWrite(5, !arrLed[cdsLevel][4]);
  digitalWrite(6, !arrLed[cdsLevel][3]);
  digitalWrite(7, !arrLed[cdsLevel][2]);
  digitalWrite(8, !arrLed[cdsLevel][1]);
  digitalWrite(9, !arrLed[cdsLevel][0]);
}

void turnOffLed(){
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
}

void waterPump(int sec){
  digitalWrite(10, LOW);
  delay(sec*1000);
  digitalWrite(10, HIGH);
}

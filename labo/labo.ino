#define BTN_PIN 2

#include <LCD_I2C.h>
LCD_I2C lcd(0x27, 16, 2);
uint8_t customCharNum[8] = {
  0b01111,
  0b01000,
  0b00110,
  0b00001,
  0b11110,
  0b00100,
  0b01000,
  0b10000,
};

const int SETUP_DELAY = 1500;
const int PIN_LED = 8;
const int PHOTO_RES_PIN = A0;
const int JS_PIN_INPUT = 2;
const int X_INPUT = A6;
const int Y_INPUT = A7;
const int INPUT_DELAY = 10;
const int REGULAR_UPDATE_DELAY = 200;
const int LIGHT_BUFFER = 5000;
const int SUEIL_LUM = 50;
const int MAX_SPEED = 120;
const int DEFAULT_SPEED = 0;
const int MIN_SPEED = -25;
const int MAX_FORWARD = 490;
const int MIN_BACKWARDS = 505;
const int MAX_BACKWARDS = 1000;
const int MIN_LEFT = -90;
const int MAX_RIGHT = 90;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  setupState();

  pinMode(PIN_LED, OUTPUT);
  pinMode(JS_PIN_INPUT, INPUT_PULLUP);
}
void setupState() {
  lcd.print("Sirois");

  lcd.createChar(1, customCharNum);

  lcd.setCursor(0, 1);
  lcd.write(1);

  lcd.setCursor(14, 1);
  lcd.print("57");

  delay(SETUP_DELAY);

  lcd.clear();
}
void speedScreenTask(int speed, int direction) {
  unsigned long CT = millis();
  static unsigned long LT = 0;

  if (CT - LT > REGULAR_UPDATE_DELAY) {
    lcd.clear();

    lcd.print("Vitesse:");

    lcd.print(speed);
    lcd.setCursor(12, 0);
    lcd.print("km/h");

    lcd.setCursor(0, 1);
    lcd.print("Direction : ");

    if (direction) {
      lcd.print("G");
    } else {
      lcd.print("D");
    }
    LT = CT;
  }
}
void speedState(int rawX, int rawY) {
  int angleX = map(rawX, 0, 1023, MIN_LEFT, MAX_RIGHT);
  int speed;
  int direction = 0;

  //handle l'axe Y
  if (rawY > MIN_BACKWARDS) {
    speed = map(rawY, MIN_BACKWARDS, MAX_BACKWARDS, DEFAULT_SPEED, MIN_SPEED); //mon joystick marcche mal et ne va pas jusqu'à 1023 :(
  } else if (rawY < MAX_FORWARD) {
    speed = map(rawY, 0, MAX_FORWARD, MAX_SPEED, DEFAULT_SPEED);
  } else {
    speed = DEFAULT_SPEED;
  }

  //handle l'axe X
  if (angleX < 0) {
    direction = !direction;
  }  // si l'angle est inférieur à 0,changer la direction pour 1

  speedScreenTask(speed, direction);
}
void lightScreenTask(int luminosite, int ledState) {
  unsigned long CT = millis();
  static unsigned long LT = 0;

  if (CT - LT > REGULAR_UPDATE_DELAY) {
    lcd.clear();

    lcd.print("Luminosite : ");
    lcd.print(luminosite);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("lumieres : ");


    if (luminosite > SUEIL_LUM) {lcd.print("OFF");} 
    else{lcd.print("ON");}

    LT = CT;
  }
}
int ledTask(unsigned long CT, unsigned long LT) {
  if (CT - LT > LIGHT_BUFFER) {
    return HIGH;
  }
}
void lightsState(unsigned long CT, int luminosite) {
  static int ledState = 0;
  static int LT;
  int lowLight = (luminosite < SUEIL_LUM);
  

  if (lowLight) {
    ledState = ledTask(CT, LT);
  } else {
    ledState = LOW;
    LT = millis();
  }
  digitalWrite(PIN_LED, ledState);
  lightScreenTask(luminosite, ledState);
}
void regularUpdate(unsigned long CT, int rawX, int rawY, int systState) {
  static unsigned long LT = 0;

  if (CT - LT > REGULAR_UPDATE_DELAY) {
    Serial.print("etd : 6229147 - ");
    Serial.print(" X: ");
    Serial.print(rawX);
    Serial.print(" Y: ");
    Serial.print(rawY);
    Serial.print(" syst: ");
    Serial.println(systState);

    LT = CT;
  }
}
int systemSwitch(unsigned long CT, int buttonPressed, int lastButtonState) {
  
  static int systState = 0;


  if (buttonPressed && buttonPressed != lastButtonState) {  //if the input delay is up
    systState = !systState;
  }

  return systState;
}
int readJSButton(){
  return digitalRead(JS_PIN_INPUT);
}
int readJSX(){
  return analogRead(X_INPUT);
}
int readJSY(){
  return analogRead(Y_INPUT);
}
int readPhotoRes(){
  return map(analogRead(A0), 0, 1023, 0, 100);
}
void loop() {
  // put your main code here, to run repeatedly:
  unsigned long CT = millis();  //current time
  static int systState = 0;
  static int lastButtonState = 0;

  const int buttonInput = readJSButton();
  const int rawX = readJSX();
  const int rawY = readJSY();
  const int luminosite = readPhotoRes();
  
  
  systState = systemSwitch(CT, buttonInput, lastButtonState);
  regularUpdate(CT, rawX, rawY, systState);
  
  if (systState) {
    lightsState(CT, luminosite);
  } else {
    speedState(rawX, rawY);
  }

  lastButtonState = buttonInput;
}


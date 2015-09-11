/* 
 *  SimoneR2's project, hosted on instructables.
 *  Last revision: 11/09/2015 dd/mm/yyyy
 *  software version: 3.0
 *  hardware version: 2.0
 *  commenti più approfonditi solo nello sketch inglese (arriveranno presto anche qui)
 */
#include <LiquidCrystal.h>
LiquidCrystal lcd ( 0, 1, 5, 6, 7, 8); //LCD PIN
#define lcdBackLight 10 //LCD Back Light Pin
#define relApertura  9 //Triac that opens the shutter
#define relChiusura  13 //Triac that close the shutter
#define tempSensor  A0 //Output of the temperature sensor
#define modeSwitch  A3 //Mode button (to choose manual or automatic operation and to turn on BackLight)
#define limiSwitch  2 //Reed relay output
#define upManual  3 //Button to open the shutter (MANUAL MODE ONLY)
#define downManual  A1 //Button to close the shutter (MANUAL MODE ONLY)
#define buzzer 4 //Emergency led and buzzer pin 
#define emergencyTemp 90 //Emergency temperature
int threshold [4] = {50, 60, 70, 80}; //insert here threshold temperature value, the last value equals shade closed. Temp in °C.
unsigned long interval = 60000; //time that the cycle don't move the shade
unsigned long BackPeriod = 30000; //BackLight duration

//You don't need to change these value------------------
int shadePosLcd = 0;
int shadePosOld = 4; //Old shade position
int shadePos = 0;    //Current shade position
int shadeMov = 0;  //Difference between the wanted position and the last position
int tempLcd = 0;  //Value to be printed on lcd
int direzione = 0; //Store the shade direction (1 = open, 0 = stop, -1 = close)
float temp = 0;      //Current temperature
unsigned long stepTime = 0; //time between a step moviment of the shade
unsigned long tempo = 0; //time misured that shade needs to close totally
unsigned long MillisMisurazione = 0; //Store shade time
unsigned long previousMillisMisurazione = 0; //Store old shade time
unsigned long previousMillis = 0; //
unsigned long Period = 0; //
unsigned long previousMillisA = 0; //LCD BackLight timer
unsigned long MillisA = 0; //LCD BackLight timer
unsigned long currentMillis = 0; //
unsigned long previousMillisB = 0; //
unsigned long MillisB = 0; //
boolean LimSwitch = 1; //Store limit switch state
boolean previousSwitch = 1; //Store previous limit switch state
boolean Switch = 0;
boolean automation = 1; //Store manual/automatic mode selected by mode button, 1 = automation mode, 0 = manual mode
boolean previousAutomation = 1; //Store last manual/automatic mode selected by mode button
boolean lcdBackLightStore = 0; //Store if light is on

//------------------------------------------------------

/*-------------------------------------
position = 0 --> shade open at 100%  |
position = 1 --> shade open at 75%   |
position = 2 --> shade open at 50%   |
posizion = 3 --> shade open at 25%   |
posizion = 4 --> shade open at 0%    |
---------------------------------------
*/
void setup() {
  BoardInit();
  digitalWrite(lcdBackLight, HIGH);
  lcd.begin(16, 2);
  lcd.print("  Solar Panel   ");
  lcd.setCursor(0, 1);
  lcd.print("roll up shutter.");
  digitalWrite(buzzer, HIGH);
  delay(400);
  digitalWrite(buzzer, LOW);
  delay(1700);
  lcd.clear();
  for (int c = 0; c <= 2; c++) {
    digitalWrite(relApertura, HIGH);
    digitalWrite(relChiusura, LOW);
    lcd.clear();
    lcd.print(" tapparella in  ");
    lcd.setCursor(0, 1);
    lcd.print(" apertura   per ");
    delay(2000);
    lcd.clear();
    lcd.print("  misurazione   ");
    lcd.setCursor(0, 1);
    lcd.print(" tempo chiusura ");
    delay(2000);
    lcd.clear();
    lcd.print("  attendere...  ");
    delay(2000);

  }
  previousMillisMisurazione = millis();
  digitalWrite(relApertura, LOW);
  delay(100);
  chiusura();
  MillisMisurazione = millis();
  tempo = MillisMisurazione - previousMillisMisurazione;
  lcd.clear();
  lcd.print(" tempo discesa: ");
  lcd.setCursor(0, 1);
  lcd.print(tempo);
  stepTime = tempo / 4;
  shadePosOld = 4;
  shadePosLcd = 100;
  digitalWrite(relChiusura, LOW);
  digitalWrite(relApertura, LOW);
  delay(1000);
  lcd.clear();
  lcd.print( "system ready! ");
  delay(600);
  lcd.setCursor(0,1);
  lcd.print("automatic mode.");
  delay(600);
  digitalWrite(lcdBackLight, LOW);
}

void loop() {
  lcd.begin(16, 2);
  temp = analogRead(tempSensor);
  temp = temp / 9.31;
  tempLcd = (int) temp;
  lcd.clear();
  lcd.print("temperatura: ");
  lcd.setCursor(12, 0);
  lcd.print(temp, DEC);
  if (temp >= emergencyTemp) {
    digitalWrite(buzzer, HIGH);
  }
  if (temp < emergencyTemp) {
    digitalWrite(buzzer, LOW);
  }
  if (automation == HIGH) {
    automatic();
  }
  if (automation == LOW) {
    manual();
    lcd.setCursor(0, 1);
    lcd.print("   manual mode  ");
  }
  delay(200);
  MillisA = millis();
  Switch = digitalRead (modeSwitch);
  //  ---------------------------------------------------------------------------------------------------------------------
  // | if modeSwitch is pressed when the display backlight is off, this is set to on, if backlight is already on you want  |
  // | to change che operation mode so it switchs to automatic or manual mode. In automatic mode the shade is controlled |
  // | by the temperature, in manual mode you command it with the two switch.                                              |
  //  ---------------------------------------------------------------------------------------------------------------------
  if ((Switch == 0) && (previousSwitch == 1)) {
    if (lcdBackLightStore == HIGH) {
      automation = !automation;
      previousSwitch = 0;
    }
    if (lcdBackLightStore == LOW) {
      digitalWrite(lcdBackLight, HIGH);
      previousMillisA = MillisA;
      previousSwitch = 0;
      lcdBackLightStore = HIGH;
      delay(1);
    }
  }
  if ((Switch == 1) && (previousSwitch == 0)) {
    previousSwitch = 1;
  }
  if (MillisA - previousMillisA >= BackPeriod) {
    digitalWrite (lcdBackLight, LOW);
    lcdBackLightStore = LOW;
  }
  if (MillisA <= 300000) {
  }
}

void BoardInit() {
  analogReference(INTERNAL); //LM35 output for our use is maximum at 1V, so 1,1V reference let us more precision than 5V reference.
  lcd.begin(16, 2);
  pinMode(relApertura, OUTPUT);
  pinMode(relChiusura, OUTPUT);
  pinMode(lcdBackLight, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(tempSensor, INPUT);
  pinMode(limiSwitch, INPUT);
  pinMode(upManual, INPUT);
  pinMode(downManual, INPUT);
  pinMode(modeSwitch, INPUT);
}
void manual() {
  previousAutomation = LOW;
  while ((digitalRead(upManual)) == 0) {
    digitalWrite(relApertura, HIGH);
    digitalWrite(relChiusura, LOW);

  }
  while ((digitalRead(downManual)) == 0) {
    digitalWrite(relApertura, LOW);
    digitalWrite(relChiusura, HIGH);

    if ((digitalRead(limiSwitch)) == 0) {
      lcd.setCursor(0, 1);
      lcd.print("tapparella chiusa");
    }
  }
  if (((digitalRead(upManual)) == 1) && ((digitalRead(downManual)) == 1)) {
    digitalWrite(relApertura, LOW);
    digitalWrite(relChiusura, LOW);
    delay(100);
  }
}
void automatic() {
  if (previousAutomation == LOW) {
    lcd.clear();
    lcd.print("automatic mode");
    delay(1000);
    chiusura();
    shadePosOld = 4;
  }
  previousAutomation = HIGH;
  currentMillis = millis();
  if ((direzione == 1) || (direzione == -1)) {
    if (currentMillis - previousMillis >= Period) {
      digitalWrite(relChiusura, LOW);
      digitalWrite(relApertura, LOW);
      direzione = 0;
    }
  }
  MillisB = millis();
  if (MillisB - previousMillisB >= interval) {
    previousMillisB = MillisB;
    if (temp < threshold[0])  {
      shadePos = 0;
      shadePosLcd = 0;
    }
    if ((temp > threshold[0]) && (temp < threshold[1])) {
      shadePos = 1;
      shadePosLcd = 25;
    }
    if ((temp > threshold[1]) && (temp < threshold[2])) {
      shadePos = 2;
      shadePosLcd = 50;
    }
    if ((temp > threshold[2]) && (temp < threshold[3])) {
      shadePos = 3;
      shadePosLcd = 75;
    }
    if (temp > threshold[3]) {
      shadePos = 4;
      shadePosLcd = 100;
    }
    shadeMov = shadePos - shadePosOld;
    shadePosOld = shadePos; //storing the shade position
    if (shadePos == 4) {
      chiusura();
      digitalWrite(relChiusura, LOW);
      direzione = 0;
      shadeMov = 0;
      shadePos = 4;
      shadePosOld = 4;
    }
    if (shadeMov == 0) {}
    if ((shadeMov < 0) && (direzione != -1)) {
      digitalWrite(relChiusura, LOW);
      digitalWrite(relApertura, HIGH);
      Period = stepTime * abs(shadeMov);
      previousMillis = currentMillis;
      direzione = 1;

    }
    if ((shadeMov > 0) && (direzione != 1)) {
      digitalWrite(relApertura, LOW);
      digitalWrite(relChiusura, HIGH);
      Period = stepTime * shadeMov;
      previousMillis = currentMillis;
      direzione = -1;
    }
  }
  lcd.setCursor(0, 1);
  lcd.print("tapparella: ");
  lcd.setCursor(12, 1);
  lcd.print(shadePosLcd, DEC);
}
void chiusura() {
  LimSwitch = digitalRead(limiSwitch);
  while (LimSwitch == 1) {
    LimSwitch = digitalRead(limiSwitch);
    digitalWrite(relChiusura, HIGH);
    digitalWrite(relApertura, LOW);
    lcd.clear();
    lcd.print(" Tapparella in ");
    lcd.setCursor(0, 1);
    lcd.print("    chiusura   ");
    delay(50);

  }
  digitalWrite(relChiusura, LOW);
}


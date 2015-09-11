/* 
 *  SimoneR2's project, hosted on instructables.
 *  Last revision: 11/09/2015 dd/mm/yyyy
 *  software version: 3.0
 *  hardware version: 2.0
 *  english version
 */
#include <LiquidCrystal.h>
LiquidCrystal lcd ( 0, 1, 5, 6, 7, 8); //LCD PIN
#define lcdBackLight 10 //LCD Back Light Pin
#define triacOpen  9 //Triac that opens the shutter
#define triacClose  13 //Triac that close the shutter
#define tempSensor  A0 //Output of the temperature sensor
#define modeSwitch  A3 //Mode button (to choose manual or automatic operation and to turn on BackLight)
#define limiSwitch  2 //Reed relay output
#define upManual  3 //Button to open the shutter (MANUAL MODE ONLY)
#define downManual  A1 //Button to close the shutter (MANUAL MODE ONLY)
#define buzzer 4 //Emergency led and buzzer pin 
#define emergencyTemp 90 //Emergency temperature
int threshold [4] = {50, 60, 70, 80}; //insert here threshold temperature value, the last value equals shutter closed. Temp in °C.
unsigned long interval = 60000; //time that the cycle don't move the shutter
unsigned long BackPeriod = 30000; //BackLight duration

//You don't need to change these value------------------
int shutterPosLcd = 0;
int shutterPosOld = 4; //Old shutter position
int shutterPos = 0;    //Current shutter position
int shutterMov = 0;  //Difference between the wanted position and the last position
int tempLcd = 0;  //Value to be printed on lcd
int direzione = 0; //Store the shutter direction (1 = open, 0 = stop, -1 = close)
float temp = 0;      //Current temperature
unsigned long stepTime = 0; //time between a step moviment of the shutter
unsigned long tempo = 0; //time misured that shutter needs to close totally
unsigned long MillisMisurazione = 0; //Store shutter time
unsigned long previousMillisMisurazione = 0; //Store old shutter time
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
position = 0 --> shutter open at 100%  |
position = 1 --> shutter open at 75%   |
position = 2 --> shutter open at 50%   |
posizion = 3 --> shutter open at 25%   |
posizion = 4 --> shutter open at 0%    |
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
    digitalWrite(triacOpen, HIGH);
    digitalWrite(triacClose, LOW);
    lcd.clear();
    lcd.print("Opening shutter ");
    lcd.setCursor(0, 1);
    lcd.print(" for measuring  ");
    delay(2000);
    lcd.clear();
    lcd.print("  closing time  ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    delay(2000);
    lcd.clear();
    lcd.print(" please wait... ");
    delay(2000);

  }
  previousMillisMisurazione = millis();
  digitalWrite(triacOpen, LOW);
  delay(100);
  chiusura();
  MillisMisurazione = millis();
  tempo = MillisMisurazione - previousMillisMisurazione;
  lcd.clear();
  lcd.print("closing time: ");
  lcd.setCursor(0, 1);
  lcd.print(tempo);
  stepTime = tempo / 4;
  shutterPosOld = 4;
  shutterPosLcd = 100;
  digitalWrite(triacClose, LOW);
  digitalWrite(triacOpen, LOW);
  delay(1000);
  lcd.clear();
  lcd.print( "system ready! ");
  delay(600);
  lcd.setCursor(0,1);
  lcd.print("automatic mode.");
  delay(1000);
  digitalWrite(lcdBackLight, LOW);
}

void loop() {
  lcd.begin(16, 2);
  temp = analogRead(tempSensor);
  temp = temp / 9.31;
  tempLcd = (int) temp;
  lcd.clear();
  lcd.print("temperature: ");
  lcd.setCursor(12, 0);
  lcd.print(temp, DEC);
  //If temperature is too high buzzer starts "buzzing"
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
  // | to change che operation mode so it switchs to automatic or manual mode. In automatic mode the shutter is controlled |
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
  analogReference(INTERNAL); //LM35's maximum output at 110°C is 1,1V so 1,1V reference is enought
  lcd.begin(16, 2);
  pinMode(triacOpen, OUTPUT);
  pinMode(triacClose, OUTPUT);
  pinMode(lcdBackLight, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(tempSensor, INPUT);
  pinMode(limiSwitch, INPUT);
  pinMode(upManual, INPUT);
  pinMode(downManual, INPUT);
  pinMode(modeSwitch, INPUT);
}
/*------------------------------------*
 * Subroutine for manual mode control *
 *------------------------------------*/
void manual() {
  previousAutomation = LOW;
  while ((digitalRead(upManual)) == 0) {
    digitalWrite(triacOpen, HIGH);
    digitalWrite(triacClose, LOW);

  }
  while ((digitalRead(downManual)) == 0) {
    digitalWrite(triacOpen, LOW);
    digitalWrite(triacClose, HIGH);

    if ((digitalRead(limiSwitch)) == 0) {
      lcd.setCursor(0, 1);
      lcd.print("shutter is close");
    }
  }
  if (((digitalRead(upManual)) == 1) && ((digitalRead(downManual)) == 1)) {
    digitalWrite(triacOpen, LOW);
    digitalWrite(triacClose, LOW);
    delay(100);
  }
}
//subroutine to adjust automatically shutter position
void automatic() {
  /*-----------------------------------------------------------------
  * If the shutter has been moved manually, then we need to recover  *
  * it's position, the only mode is closing totally the shutter.     *
  -------------------------------------------------------------------*/
  if (previousAutomation == LOW) {
    lcd.clear();
    lcd.print("automatic mode"); //confirm automatic mode selection (user notification)
    delay(1000);
    chiusura();
    shutterPosOld = 4; //The shutter position is now 4 (100% closed)
  }
  previousAutomation = HIGH; 
  currentMillis = millis();
  /*---------------------------------------------------------------
   * If the shutter is moving but the moving time has expired, we *
   * need to stop it because it has reach the right position.     *
   ---------------------------------------------------------------*/
  if ((direzione == 1) || (direzione == -1)) {
    if (currentMillis - previousMillis >= Period) {
      digitalWrite(triacClose, LOW);
      digitalWrite(triacOpen, LOW);
      direzione = 0;
    }
  }
  MillisB = millis();
  /*-----------------------------------------------------------
   * If it's time to move the shutter, we have to extabilish   *
   * shutter position depending on the temperature             *
   ------------------------------------------------------------*/
  if (MillisB - previousMillisB >= interval) {
    previousMillisB = MillisB;
    if (temp < threshold[0])  {
      shutterPos = 0;
      shutterPosLcd = 0;
    }
    if ((temp > threshold[0]) && (temp < threshold[1])) {
      shutterPos = 1;
      shutterPosLcd = 25;
    }
    if ((temp > threshold[1]) && (temp < threshold[2])) {
      shutterPos = 2;
      shutterPosLcd = 50;
    }
    if ((temp > threshold[2]) && (temp < threshold[3])) {
      shutterPos = 3;
      shutterPosLcd = 75;
    }
    if (temp > threshold[3]) {
      shutterPos = 4;
      shutterPosLcd = 100;
    }
    /*-------------------------------------------------
     * Here we calculate how many step we need to move *
     * the shutter.                                    *
     -------------------------------------------------*/
    shutterMov = shutterPos - shutterPosOld;
    shutterPosOld = shutterPos; //storing the shutter position for next cycle
    /*------------------------------------------------------
    * If shutter's position is request as totally closed,   *
    * then we closeit with limit switch.                    *
    --------------------------------------------------------*/
    if (shutterPos == 4) {
      chiusura();
      digitalWrite(triacClose, LOW);
      direzione = 0;
      shutterMov = 0;
      shutterPos = 4;
      shutterPosOld = 4;
    }
    if (shutterMov == 0) {}
    if ((shutterMov < 0) && (direzione != -1)) {
      digitalWrite(triacClose, LOW);
      digitalWrite(triacOpen, HIGH);
      Period = stepTime * abs(shutterMov);
      previousMillis = currentMillis;
      direzione = 1;

    }
    if ((shutterMov > 0) && (direzione != 1)) {
      digitalWrite(triacOpen, LOW);
      digitalWrite(triacClose, HIGH);
      Period = stepTime * shutterMov;
      previousMillis = currentMillis;
      direzione = -1;
    }
  }
  lcd.setCursor(0, 1);
  lcd.print("shutter at: ");
  lcd.setCursor(12, 1);
  lcd.print(shutterPosLcd, DEC);
} 

//Subroutine to close the shutter completly
void chiusura() {
  LimSwitch = digitalRead(limiSwitch);
  while (LimSwitch == 1) {
    LimSwitch = digitalRead(limiSwitch);
    digitalWrite(triacClose, HIGH);
    digitalWrite(triacOpen, LOW);
    lcd.clear();
    lcd.print("closing shutter");
    lcd.setCursor(0, 1);
    lcd.print("    wait...    ");
    delay(50);

  }
  digitalWrite(triacClose, LOW);
}


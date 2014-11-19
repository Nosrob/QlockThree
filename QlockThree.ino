#include <Time.h>  
#include <LedControl.h>
#include <binary.h>
#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

// LEDs stuff
const int LC1CLK   = 13;
const int LC1LOAD  = 12;
const int LC1DATA  = 11;
const int LC2CLK   = 10;
const int LC2LOAD  = 9;
const int LC2DATA  = 8;
LedControl LC1=LedControl(LC1DATA, LC1CLK, LC1LOAD, 1); 
LedControl LC2=LedControl(LC2DATA, LC2CLK, LC2LOAD, 1); 
int ledBottomLeft=14;
int ledBottomRight=15;
int ledTopRight=16;
int ledTopLeft=17;
const int ledDelay = 200;
unsigned long ledLastUpdate = 0;
int ledIntensity = 15;


// Buttons stuff
const int BUT1 = 2; // Brightness
const int BUT2 = 3; // +1 minute
const int BUT3 = 4; // +5 minutes
const int BUT4 = 5; // +60 minutes (1 hour)
unsigned long but1LastPress = 0;  
unsigned long but2LastPress = 0;  
unsigned long but3LastPress = 0;  
unsigned long but4LastPress = 0;  
const int buttonPressDelay = 200;
long startTime; // the value returned from millis when the switch is pressed
long duration;  // variable to store the duration
int buttonState = 0;

//Modes stuff
const int MODE_DEFAULT = 0;
const int MODE_SECONDS = 1;
const int MODE_NOITIS = 2;
const int MODE_SCROLLER = 3;
int currentMode;

//Time displayed on the clock
int cHour = 0;
int cMin = 0;
int cSec = 0;

boolean forceUpdate = true;

// the setup routine runs once when you press reset:
void setup() { 

	//Initiate LED corners
	pinMode(ledBottomLeft, OUTPUT);
	pinMode(ledBottomRight, OUTPUT);
	pinMode(ledTopRight, OUTPUT);
	pinMode(ledTopLeft, OUTPUT);

	// Clear LED Display  
	LC1.shutdown(0,false);
	LC1.clearDisplay(0);
	LC1.setIntensity(0,ledIntensity);  
	LC2.shutdown(0,false);
	LC2.clearDisplay(0);
	LC2.setIntensity(0,ledIntensity);

	// Initiate Serial connection
	Serial.begin(9600);

	// Initiate Buttons
	pinMode(BUT1, INPUT);     
	pinMode(BUT2, INPUT);     
	pinMode(BUT3, INPUT);     
	pinMode(BUT4, INPUT);     

	// Initiate time
	setSyncProvider(RTC.get);   // the function to get the time from the RTC
	setSyncInterval(5);
	if (timeStatus() != timeSet) 
		Serial.println("Unable to sync with the RTC");
	else
		Serial.println("RTC has set the system time");

	// Displays the time it is...
	currentMode = MODE_DEFAULT;
}

// the loop routine runs over and over again forever:
void loop() {

	//Debug, prints the time in Serial connection
	// Serial.println();
	// print2digits(hour());
	// Serial.write(':');
	// print2digits(minute());
	// Serial.write(':');
	// print2digits(second());

	// Anyone pressed a button?
	int but1read = digitalRead(BUT1);
	int but2read = digitalRead(BUT2);
	int but3read = digitalRead(BUT3);
	int but4read = digitalRead(BUT4);

	if (but1read == HIGH){
		startTime=millis();

		while(digitalRead(BUT1)==HIGH){
			if ((millis() - startTime) > 3000) {
				Serial.print("Changing mode to: Seconds Mode");
				currentMode = MODE_SECONDS;  
			}     
		}
		duration = millis() - startTime;
		if (((millis() - but1LastPress) > buttonPressDelay) && duration < 3000) {
			doButton1();
		}
		but1LastPress = millis();
	} 
	else if ((but1read == LOW) && ((millis() - but1LastPress) > buttonPressDelay )){
		but1LastPress = 0;  // reset
	}


	if ((but2read == HIGH) && ((millis() - but2LastPress) > buttonPressDelay)) {
		but2LastPress = millis();
		doButton2();
	}
	else if ((but2read == LOW) && ((millis() - but2LastPress) > buttonPressDelay )){
		but2LastPress = 0;  // reset
	}

	if ((but3read == HIGH) && ((millis() - but3LastPress) > buttonPressDelay)) {
		but3LastPress = millis();
		doButton3();
	}
	else if ((but3read == LOW) && ((millis() - but3LastPress) > buttonPressDelay )){
		but3LastPress = 0;  // reset
	}

	if (but4read == HIGH){
		startTime=millis();

		while(digitalRead(BUT4)==HIGH){
			if ((millis() - startTime) > 3000) {
				Serial.print("Changing mode to: No *IT IS* Mode");
				currentMode= MODE_NOITIS;
			}     
		}

		duration = millis() - startTime;
		if (((millis() - but4LastPress) > buttonPressDelay) && duration < 3000) {
			doButton4();
		}
		but4LastPress = millis();
	} 
	else if ((but4read == LOW) && ((millis() - but4LastPress) > buttonPressDelay )){
		but4LastPress = 0;  // reset
	}

	// If no button was pressed
	if ((millis() - ledLastUpdate) > ledDelay) {
		ledLastUpdate = millis();
		if (currentMode == MODE_DEFAULT) mode_default();
		// else if (currentMode == MODE_SECONDS) mode_defaultsec();
		// else if (currentMode == MODE_NOITIS) mode_seconds();
	}
}

// LED Turn on/off procedures
void LED_CLEAR() {
	LC1.clearDisplay(0);
	LC2.clearDisplay(0);
}

// Default mode
void mode_default() {

	//Real time now  
	int hourNow = hour();
	int minuteNow = minute();
	int secondNow = second();

	//Quit if time has not changed yet
	if ((hourNow == cHour) && (minuteNow == cMin) && (forceUpdate == false))
		return;

	// Time to display
	int tpast5mins = minuteNow % 5; // remainder
	int t5mins = minuteNow - tpast5mins;
	int tHour = hourNow;

	if (tHour > 12) tHour = tHour - 12;
	//else if (tHour == 0) tHour = 12;

	LED_CLEAR();
	W_ITIS();

	// past?
	if (t5mins > 30){
		W_TO();
		tHour = tHour+1;
		if (tHour > 12) tHour = 1;
	}

	if (tHour == 0) H_MIDNIGHT(); 
	else if (tHour == 1) H_ONE(); 
	else if (tHour == 2) H_TWO(); 
	else if (tHour == 3) H_THREE(); 
	else if (tHour == 4) H_FOUR();
	else if (tHour == 5) H_FIVE(); 
	else if (tHour == 6) H_SIX(); 
	else if (tHour == 7) H_SEVEN(); 
	else if (tHour == 8) H_EIGHT();
	else if (tHour == 9) H_NINE(); 
	else if (tHour == 10) H_TEN(); 
	else if (tHour == 11) H_ELEVEN(); 
	else if (tHour == 12) H_NOON();


	if (t5mins == 5 || t5mins == 55)     M_FIVE();
	else if (t5mins == 10 || t5mins == 50)    M_TEN();
	else if (t5mins == 15)    M_QUARTER_PLUS();
	else if (t5mins == 45)    M_QUARTER_MINUS();
	else if (t5mins == 20 || t5mins == 40)    M_TWENTY();
	else if (t5mins == 25 || t5mins == 35)    M_TWENTYFIVE();
	else if (t5mins == 30)    M_HALF();

	// +1min leds in the corners...
	if (tpast5mins == 0 ) { 
		P_ONE_OFF();
		P_TWO_OFF(); 
		P_THREE_OFF(); 
		P_FOUR_OFF();
	}
	else if (tpast5mins == 1) { 
		P_ONE_ON(); 
		P_TWO_OFF(); 
		P_THREE_OFF(); 
		P_FOUR_OFF();
	}
	else if (tpast5mins == 2) { 
		P_ONE_ON(); 
		P_TWO_ON(); 
		P_THREE_OFF(); 
		P_FOUR_OFF();
	}
	else if (tpast5mins == 3) { 
		P_ONE_ON(); 
		P_TWO_ON(); 
		P_THREE_ON(); 
		P_FOUR_OFF();
	}
	else if (tpast5mins == 4) { 
		P_ONE_ON(); 
		P_TWO_ON(); 
		P_THREE_ON(); 
		P_FOUR_ON(); 
	}

	W_HOURS();

	// save last updated time
	cHour = hourNow;
	cMin = minuteNow;
	cSec = secondNow;
	forceUpdate = false;
}

// Brightness
void doButton1() {
	if (ledIntensity >= 15){
		ledIntensity=0;
	}
	else {
		ledIntensity = ledIntensity + 3;
	}

	LC1.setIntensity(0,ledIntensity);    
	LC2.setIntensity(0,ledIntensity);    
}

// +1 min
void doButton2() {
	RTC.set(now()+60-second());
	setTime(RTC.get());
}

// +5 min
void doButton3() {
	RTC.set(now()+300-second());
	setTime(RTC.get());
}

// +60 min
void doButton4() {
	RTC.set(now()+3600-second());
	setTime(RTC.get());
}

// Used for Serial debug
void print2digits(int number) {
	if (number >= 0 && number < 10) {
		Serial.write('0');
	}
	Serial.print(number);
}

// IL EST
void W_ITIS() {
	LC1.setRow(0,0,B11011100);
}

// CINQ
void M_FIVE(){
	LC2.setRow(0,3,B00000011);
	LC2.setRow(0,5,B00100000);
	LC2.setRow(0,6,B00100000);  
}

// DIX
void M_TEN(){
	LC2.setRow(0,5,B00001000);  
	LC2.setRow(0,6,B00001000); 
	LC2.setRow(0,7,B00001000); 
}

// ET QUART
void M_QUARTER_PLUS(){
	LC2.setRow(0,2,B11011111);
}

// MOINS LE QUART
void M_QUARTER_MINUS(){
	LC2.setRow(0,1,B11111011);
	LC2.setRow(0,2,B00011111);
}

// VINGT
void M_TWENTY(){
	LC2.setRow(0,3,B11111000);
}

// VINGT-CINQ
void M_TWENTYFIVE(){
	LC2.setRow(0,3,B11111111);
	LC2.setRow(0,5,B00100000);
	LC2.setRow(0,6,B00100000);  
}

// ET DEMIE
void M_HALF(){
	LC2.setRow(0,4,B11011111);
}

// UNE
void H_ONE(){
	LC1.setRow(0,2,B00001110);
}
// DEUX
void H_TWO(){
	LC1.setLed(0,0,7,true); 
	LC1.setRow(0,5,B00000100);  
	LC1.setRow(0,6,B00000100); 
	LC1.setRow(0,7,B00000100); 
}

// TROIS
void H_THREE(){
	LC1.setLed(0,1,7,true); 
	LC1.setLed(0,1,6,true); 
	LC1.setRow(0,5,B00001000);  
	LC1.setRow(0,6,B00001000); 
	LC1.setRow(0,7,B00001000);
}

// QUATRE
void H_FOUR(){
	LC1.setRow(0,1,B11111100);
}

// CINQ
void H_FIVE(){
	LC1.setLed(0,3,7,true); 
	LC1.setRow(0,5,B00100000);  
	LC1.setRow(0,6,B00100000); 
	LC1.setRow(0,7,B00100000); 
}

// SIX
void H_SIX(){
	LC1.setRow(0,3,B00001110); 
}

// SEPT
void H_SEVEN(){
	LC1.setLed(0,2,7,true); 
	LC1.setRow(0,5,B00010000);  
	LC1.setRow(0,6,B00010000); 
	LC1.setRow(0,7,B00010000);
}

// HUIT
void H_EIGHT(){
	LC1.setRow(0,3,B11110000);
}

// NEUF
void H_NINE(){
	LC1.setRow(0,2,B11110000);
}

// DIX
void H_TEN(){
	LC1.setRow(0,4,B00111000);
}

// ONZE
void H_ELEVEN(){
	LC2.setRow(0,0,B11110000);
}

// MIDI
void H_NOON(){
	LC1.setRow(0,4,B11110000);
}

// MINUIT
void H_MIDNIGHT(){
	LC1.setRow(0,4,B00000111);
	LC1.setRow(0,5,B01000000); 
	LC1.setRow(0,6,B01000000); 
	LC1.setRow(0,7,B01000000); 
}

// MOINS
void W_TO(){
	LC2.setLed(0,1,0,true);
	LC2.setLed(0,1,1,true);
	LC2.setLed(0,1,2,true);
	LC2.setLed(0,1,3,true);
	LC2.setLed(0,1,4,true); 
}

// HEURES
void W_HOURS(){
	LC2.setLed(0,0,5,true);
	LC2.setLed(0,0,6,true);
	LC2.setLed(0,0,7,true);
	LC2.setLed(0,5,5,true);
	LC2.setLed(0,6,5,true);
	LC2.setLed(0,7,5,true);
}

void P_ONE_ON(){
	digitalWrite(ledTopLeft, HIGH); 
}
void P_ONE_OFF(){
	digitalWrite(ledTopLeft, LOW); 
}

void P_TWO_ON(){
	digitalWrite(ledTopRight, HIGH); 
}
void P_TWO_OFF(){
	digitalWrite(ledTopRight, LOW); 
}

void P_THREE_ON(){
	digitalWrite(ledBottomRight, HIGH); 
}
void P_THREE_OFF(){
	digitalWrite(ledBottomRight, LOW); 
}

void P_FOUR_ON(){
	digitalWrite(ledBottomLeft, HIGH); 
}
void P_FOUR_OFF(){
	digitalWrite(ledBottomLeft, LOW); 
}


void mode_defaultsec(){
	// TODO
}

void mode_seconds(){
	// TODO
}

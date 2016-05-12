/*
 Name:		KitchenLight.ino
 Created:	12.05.2016 21:35:53
 Author:	Ant
*/

// the setup function runs once when you press reset or power the board
#include <EEPROM.h>

#define TAMPER_PIN 0
#define BOX_LIGHT_PIN 1 //pwm

#define INFRARED_SENSOR_PIN 2
#define TABLE_LIGHT_PIN 4

#define BOX_LIGHT_PWM_VALUE 180

#define TABLE_LIGHT_PWM_VALUE_EPPROM_ADDR 0
#define INFRARED_SENSOR_ACTIVE_TIME_FOR_OFF 1000 // msec
#define MAIN_LOOP_DELAY_TIME 100

byte tableLightPWMVal = 0; // actual pwm value

void setup() {
	pinMode(TAMPER_PIN, INPUT);
	pinMode(INFRARED_SENSOR_PIN, INPUT);

	pinMode(BOX_LIGHT_PIN, OUTPUT);
	pinMode(TABLE_LIGHT_PIN, OUTPUT);

	tableLightPWMVal = EEPROM.read(TABLE_LIGHT_PWM_VALUE_EPPROM_ADDR);
}

boolean isTableLightOn = false;
long infraredSensorIsActiveCount = 0; // count time while infrared sensor is active
boolean infraredSensorPrevState = HIGH; // initial state

void loop() {

	updateBoxLightState();

	boolean infraredSensorValue = digitalRead(INFRARED_SENSOR_PIN);

	boolean ifLampWillBeOff = false; // need for skip action after lamp off
	if (LOW == infraredSensorValue)
	{
		// count active state 
		infraredSensorIsActiveCount += 1;

		// check if need to turn off lamp
		if (infraredSensorIsActiveCount > (INFRARED_SENSOR_ACTIVE_TIME_FOR_OFF / MAIN_LOOP_DELAY_TIME))
		{
			// if timeout is exceeded  turn off lamp
			analogWrite(TABLE_LIGHT_PIN, 0);
			isTableLightOn = false;
			// set flag sign that lamp will be off in this iteration
			ifLampWillBeOff = true; 
		}
	} 
	else
	{
		infraredSensorIsActiveCount = 0; // reset count
	}

	if (true == ifLampWillBeOff)
	{
		// need for correct handling sensor values after lamp off
		long timeoutAfterOff = 20000; 
		boolean curInfraredSensVal = digitalRead(INFRARED_SENSOR_PIN);
		while (curInfraredSensVal == LOW) {
			curInfraredSensVal = digitalRead(INFRARED_SENSOR_PIN);
			timeoutAfterOff -= 100;
			if (timeoutAfterOff < 0)
			{
				break;
			}
			delay(100);
		}
	}
	else
	{
		if ((infraredSensorPrevState != infraredSensorValue) && (infraredSensorValue == HIGH)) // sensor off after active
		{
			int currentPWMValForTableLamp = 0;
			// choose needed value for table lamp
			if (true == isTableLightOn)
			{

				// increment pwm value in loop
				currentPWMValForTableLamp = tableLightPWMVal + 120;
				if (currentPWMValForTableLamp > 255)
				{
					currentPWMValForTableLamp = 15;
				}
				EEPROM.update(TABLE_LIGHT_PWM_VALUE_EPPROM_ADDR, currentPWMValForTableLamp);
				tableLightPWMVal = currentPWMValForTableLamp;
			}
			else
			{
				currentPWMValForTableLamp = tableLightPWMVal;
				isTableLightOn = true;
			}

			// update table lamp pwm value 
			analogWrite(TABLE_LIGHT_PIN, currentPWMValForTableLamp);

			delay(500);
		}
	}

	infraredSensorPrevState = digitalRead(INFRARED_SENSOR_PIN);

	delay(MAIN_LOOP_DELAY_TIME); // between check sensors
}

void updateBoxLightState() {
	boolean isBoxDoorOpen = digitalRead(TAMPER_PIN) == HIGH;
	
	if (true == isBoxDoorOpen) {
		analogWrite(BOX_LIGHT_PIN, BOX_LIGHT_PWM_VALUE);
	}
	else {
		analogWrite(BOX_LIGHT_PIN, 0);
	}
}

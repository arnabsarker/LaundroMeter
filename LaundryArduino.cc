#include < SoftwareSerial.h >
SoftwareSerial BTSerial(2, 3); 

//CONSTANTS
const int DRIER_TIME = 45*60; //seconds it takes to dry clothes, approx. 45 minutes
//Threshold values used to determine events for the dryer.
const int X_DOOR_THRESHOLD = 30;
const int Y_DOOR_THRESHOLD = 30;
const int Z_DOOR_THRESHOLD = 70;
const double OFF_THRESHOLD = 5;
const int ON_TEMP_THRESHOLD = 3;
//constant used to convert temperature to degrees F
const double TEMP_MULTIPLIER = 500/1024.0;

//Setup values for Arduino pins
const int xPin = 2;
const int yPin = 3;
const int zPin = 4;
const int tPin = 0;

//INITIALIZED VALUES FOR PROGRAM
//Calibration array: initialized at the beginning so both setup() and loop() can access the values
int CalVal[] = {0, 0, 0, 0}; //Stores calibrated values for x, y, and z accelerations and temperature, in that order
                              //Since jerk is calculated with calibrated accelerations, it does not need to have its own calibration.
boolean dryerstarted = false; //boolean representing whether the dryer has started. Initial condition assumes the dryer is off.
boolean dooropen = false; //boolean representing whether the dryer door is open.Initiial condition assumes door is closed. 

void setup() {
  //Begin communication with bluetooth and serial monitor.
  BTSerial.begin(9600); 
  Serial.begin(9600);

  //Calibrates Machine before beginning
  Serial.println("Hold Steady for Calibration...");
  int i = 0; 
  int xcRead = 0;
  int ycRead = 0;
  int zcRead = 0;
  int temp = 0;
  //Adds together values for each variable for 5 seconds
  while(i < 50){
    xcRead = xcRead + analogRead(xPin);
    ycRead = ycRead + analogRead(yPin);
    zcRead = zcRead + analogRead(zPin);

    temp += analogRead(tPin) * TEMP_MULTIPLIER;
    
    i = i + 1;
    delay(100);
  }
  //Stores average value for each variable into the CalVal array so they can be accessed in the loop
  CalVal[0] = xcRead / 50; //Calibrated accelerations
  CalVal[1] = ycRead / 50;
  CalVal[2] = zcRead / 50;
  CalVal[3] = temp / 50;
  Serial.println("Temperature Calibration: " + (String) CalVal[3]);
  Serial.println("x calibration: " + (String) CalVal[0] + "y calibration: " + (String)CalVal[1] + "z calibration: " + (String)CalVal[2]);
  Serial.println("Calibration done!");

}


//Initializes arrays to store acceleration values for two consecutive calculations  
int xRead[] ={0, 0};
int yRead[] ={0, 0};
int zRead[] ={0, 0};

//Starts a count of how long the dryer has been on
int halfsecondspassed = 0;

void loop() {

  //Stores previous value of acceleration in Read[0] and the current value in Read[1], so jerk can be calculated later
  xRead[0] = xRead[1]; xRead[1] = analogRead(xPin) - CalVal[0];
  yRead[0] = yRead[1]; yRead[1] = analogRead(yPin) - CalVal[1];
  zRead[0] = zRead[1]; zRead[1] = analogRead(zPin) - CalVal[2];

  //Uses these consecutive values to calculate jerk.
  int xjRead = (xRead[1] - xRead[0]);
  int yjRead = (yRead[1] - yRead[0]);
  int zjRead = (zRead[1] - zRead[0]);

  //stores difference in temperature from calculated value
  int dTemp = analogRead(tPin) * TEMP_MULTIPLIER - CalVal[3];
  
  //Print to serial monitor for debugging
  Serial.println("x: " + (String) xRead[1] + " y: " + (String) yRead[1] + "z: " + (String) zRead[1]);
  Serial.println("xjerk: " + (String) xjRead + " yjerk: " + (String) yjRead + " zjerk: " + (String) zjRead);
  Serial.println("Temp: " + (String) dTemp);
  
  //The following code determines when to send a message to the other Arduino:
  
  //Door Opened: Occurs when the accelerations reach a certain value and the door is not already open.
  if(((abs(xjRead) > X_DOOR_THRESHOLD || abs(yjRead) > Y_DOOR_THRESHOLD) && (abs(zjRead) > Z_DOOR_THRESHOLD) && !dooropen)){
    Serial.println("D");
    BTSerial.println("D");
    dooropen = true;
  }
  //Machine off: Occurs when all the accelerations are below a certain threshold (representing the machine's natural vibration)
  //and the dryer has already been started. Could also occur if the drier has been running for long enough.
  else if (((abs(xRead[1]) < OFF_THRESHOLD) && (abs(yRead[1]) < OFF_THRESHOLD) && (abs(zRead[1]) < OFF_THRESHOLD) && dryerstarted) 
             || (halfsecondspassed > DRIER_TIME * 2)){
    Serial.println("O");
    BTSerial.println("O");
    dryerstarted = false;
  } 
  //Machine on: Should occur if the laudry machine is hot enough and it isn't already on
  else if (abs(dTemp) > ON_TEMP_THRESHOLD  && !dryerstarted){
    Serial.println("N");
    BTSerial.println("N");
    dryerstarted = true;
  } else{
    Serial.println("");
    BTSerial.println("");
  } 
  delay(500); // delay so we can read the values more clearly

  //Code to keep track of time
  if(dryerstarted){
  halfsecondspassed +=1; //keeps track of the time, since each loop is approximately half a second
  } else {
    halfsecondspassed = 0; //if the dryer hasn't started, the time should be 0
  }
} 

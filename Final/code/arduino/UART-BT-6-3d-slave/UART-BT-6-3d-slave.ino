///
/// BT controlled platform for Lychee/other controllers 
// - 2 HT Rhino DC motors via UART, 2 DC motors via Digital pins 
/// This code works only on MEGA + Motor Shield
/// Requires - 
///   Serial Port2 {16 Tx,17}, Port3 {14 Tx,15}, Bluetooth Serial Port1 {18 Tx,19}
///   Serial Port0 for COM
///   DC Motor - Digital Pins 5, 6 
/// Connections - Motor Yellow Rx to Arduino Tx. HC05 Rx to Ard Tx
///
/// 2018

#include <AFMotor.h> 

// For Rhino controls
int useMotor2 = 1;
int useMotor3 = 1;
int platformDirection = +1;
int dontRunMotors = 1;
int useBluetooth = 1;
int timeMultiplier = 1;
int pickupDropSequence = 0;
int communictionLost = 0;

int ledpin = 13; // led on D13 will show blink on / off
int firstBtByte = '0'; 
int previousBtByte = 'w';
int delayMilliSec = 10;
int delaySteps = 1000;

// These are for normal DC motors via Shield
int useMotor4 = 0;
int useMotor5 = 0;
AF_DCMotor dcMotor4 (3);
AF_DCMotor dcMotor5 (4);

void setup() 
{
   // initialize serial ports:
    Serial.begin(9600);

  if(useMotor3 == 1) 
  {
    Serial3.begin(9600);
    delay(delayMilliSec);
    Serial3.println("M+255");
    delay(delayMilliSec);
    Serial3.println("D20");
    delay(delayMilliSec);        
    Serial3.println("S0");
    delay(delayMilliSec);    
  }
  
  if(useMotor2 == 1)
  {
    Serial2.begin(9600);
    delay(delayMilliSec);
    Serial2.println("M+255");
    delay(delayMilliSec);
    Serial2.println("D20");
    delay(delayMilliSec);        
    Serial2.println("S0");
    delay(delayMilliSec);    
  }

  // init Bt, after AT+ORGL
  if(1 == useBluetooth)
  {
    pinMode(ledpin,OUTPUT);
    Serial1.begin(38400);
  }

  // Regular DC
  if(useMotor4 == 1)
  {
    dcMotor4.setSpeed(150);
    dcMotor4.run(RELEASE);
  }
  if(useMotor5 == 1)
  {
    dcMotor5.setSpeed(150);
    dcMotor5.run(RELEASE);
  }
}
void loop() 
{ 
    // read from port 1, send to port 0:
    while (Serial2.available()>0) 
    {
      int inByte = Serial2.read();
      delay(delayMilliSec);
      //Serial.write(inByte);
    }
    while (Serial3.available()>0) {
      int inByte = Serial3.read();
      delay(delayMilliSec);
      //Serial.write(inByte);
    }

  // Read control via bluetooth, if bt is enabled
  if(useBluetooth == 1)
  {
    if(Serial1.available()>0)
    {
      previousBtByte = firstBtByte;
      firstBtByte = Serial1.read();
    }

    // Control the motors via Bt
    if(0)
    {
      dontRunMotors = 0;
      useMotor2 = 0;
      useMotor3 = 0;
      useMotor4 = 0;
      useMotor5 = 0;
      pickupDropSequence = 0;
      platformDirection = 1;
    }
    
    // w - Forward
    // a - Left - Motor2 ON
    // d - Right - Motor3 ON
    // s - Reverse
    // 0 - Stop
    // r - Rotate arm
    // p - Pickup (Grasp/close claw)
    // l - Leave claw
    
    // need to continuously rotate, so dont skip on 'r'
    if((firstBtByte != 'r') && (previousBtByte == firstBtByte))
    {
      // nothing to do
      //delay(10);
      //Serial.println(firstBtByte);      
      return;
    }
    else if ('w' == firstBtByte)
    {
      dontRunMotors = 0;
      useMotor2 = 1;
      useMotor3 = 1;
      platformDirection = 1;
    }
    else if ('a' == firstBtByte)
    {
      dontRunMotors = 0;
      useMotor2 = 1;
      useMotor3 = 0;  
      platformDirection = 1;          
    }
    else if ('d' == firstBtByte)
    {
      dontRunMotors = 0;
      useMotor3 = 1;
      useMotor2 = 0;
      platformDirection = 1;
    }
    else if ('s' == firstBtByte)
    {
      dontRunMotors = 0;
      useMotor2 = 1;
      useMotor3 = 1;
      platformDirection = -1;
    }    
    else if ('r' == firstBtByte)
    {
      dontRunMotors = 0;
      useMotor4 = 1;
    }
    else if ('p' == firstBtByte)
    {
      dontRunMotors = 0;
      useMotor5 = 1;
    }
    else if ('l' == firstBtByte)
    { 
      dontRunMotors = 0;
      useMotor5 = -1;
    }
    else if ('q' == firstBtByte)
    { 
      dontRunMotors = 0;
      pickupDropSequence = 1;
    }
    else if('0' == firstBtByte)
    {
      //default is stop
      dontRunMotors = 1;
      useMotor2 = 0;
      useMotor3 = 0;
    }
    else if('1' == firstBtByte)
    {
      timeMultiplier = 1;
    }
    else if('2' == firstBtByte)
    {
      timeMultiplier = 2;
    }
    else
    {
      
      communictionLost ++;
      if(communictionLost > 1000)
      {
        //if we dont receive cmd for 1000 itertions stop
        firstBtByte = '0';
        dontRunMotors = 1;
        useMotor2 = 1;
        useMotor3 = 1;
        useMotor4 = 0;
        useMotor5 = 0;
        pickupDropSequence = 0;
        platformDirection = 1;    

        communictionLost = 0;
      }
    }
  }

  // Motor control section starts
  if(dontRunMotors == 1)
  {
    Serial3.println("S0");
    Serial2.println("S0");  
    dcMotor4.run(RELEASE);
    dcMotor5.run(RELEASE);
    return;
  }

  // Set motor speeds (FORWARD - BACKWARD sequence)
  if(1 == platformDirection)
  {
    if(useMotor2 == 1) Serial2.println("S100");  else Serial2.println("S0");
    if(useMotor3 == 1) Serial3.println("S-100"); else Serial3.println("S0"); 
  }
  else // Reverse 
  {
    if(useMotor2 == 1) Serial2.println("S-100"); else Serial2.println("S0"); 
    if(useMotor3 == 1) Serial3.println("S100"); else Serial3.println("S0");
  }

  // Motor shield arm rotation
  if(useMotor4 == 1)
  {
    dcMotor4.run(FORWARD);
    delay(200 );
    dcMotor4.run(RELEASE);
  }
  if(useMotor5 == 1)
  {
    dcMotor5.run(FORWARD);
    delay(500);
    dcMotor5.run(RELEASE);    
  }
  else if(useMotor5 == -1)
  {
    dcMotor5.run(BACKWARD);
    delay(500);
    dcMotor5.run(RELEASE);    
  }

  if(pickupDropSequence == 1)
  {
    dcMotor4.run(FORWARD);
    dcMotor5.run(FORWARD);
    delay(700);
    dcMotor4.run(BACKWARD);
    dcMotor5.run(BACKWARD);
    delay(700);
    dcMotor4.run(RELEASE);    
    dcMotor5.run(RELEASE);    
  }
  delay(10);
}

// Constants for leds and delay time
const int thirdRightLine = 13;
const int secondRightLine = 11;
const int firstRightLine = 10;
const int rightFlag = 12;
const int firstRightChannel = 9;
const int secondRightChannel = 8;
const int middleChannel = 7;
const int secondLeftChannel = 6;
const int firstLeftChannel = 5;
const int firstLeftLine = 3;
const int secondLeftLine = 2;
const int thirdLeftLine = A5;
const int leftFlag = A4;
const int delayTimeConstant = 350;


// Global variables
int incomingByte = 0;   // For trash data
int delayTime;
int trafficLight = 0;
int leftLine = 0;
int rightLine = 0;
int channel = 0;
int counter = 0;
int data = 0;
bool leftFlagOn = false;
bool rightFlagOn = false;
bool dataFlag = true;

void setup() {
    Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
    pinMode(thirdRightLine, OUTPUT);
    pinMode(secondRightLine, OUTPUT);
    pinMode(firstRightLine, OUTPUT);
    pinMode(rightFlag, OUTPUT);
    pinMode(firstRightChannel, OUTPUT);
    pinMode(secondRightChannel, OUTPUT);
    pinMode(middleChannel, OUTPUT);
    pinMode(secondLeftChannel, OUTPUT);
    pinMode(firstLeftChannel, OUTPUT);
    pinMode(firstLeftLine, OUTPUT);
    pinMode(secondLeftLine, OUTPUT);
    pinMode(thirdLeftLine, OUTPUT);
    pinMode(leftFlag, OUTPUT);
}

void turnOffChannel() {
  digitalWrite(firstRightChannel, LOW);
  digitalWrite(secondRightChannel, LOW);
  digitalWrite(middleChannel, LOW);
  digitalWrite(secondLeftChannel, LOW);
  digitalWrite(firstLeftChannel, LOW);
}

void setTrafficLight() {
  switch (trafficLight) {
    case 1: // Green flag for left ships
      digitalWrite(rightFlag, LOW);
      digitalWrite(leftFlag, HIGH);
      break;
    case 2: // Green flag for right ships
      digitalWrite(rightFlag, HIGH);
      digitalWrite(leftFlag, LOW);
      break;
    default: // No green flag
      digitalWrite(rightFlag, LOW);
      digitalWrite(leftFlag, LOW);
      break;
  }
  
}

void setLeftLine() {
  switch (leftLine) {
    case 0:
      digitalWrite(firstLeftLine, LOW);
      digitalWrite(secondLeftLine, LOW);
      digitalWrite(thirdLeftLine, LOW);
      break;
    case 1:
      digitalWrite(firstLeftLine, HIGH);
      digitalWrite(secondLeftLine, LOW);
      digitalWrite(thirdLeftLine, LOW);
      break;
    case 2:
      digitalWrite(firstLeftLine, HIGH);
      digitalWrite(secondLeftLine, HIGH);
      digitalWrite(thirdLeftLine, LOW);
      break;
    case 3:
      digitalWrite(firstLeftLine, HIGH);
      digitalWrite(secondLeftLine, HIGH);
      digitalWrite(thirdLeftLine, HIGH);
      break;
    default: // If more than 3 ships, third led blinks
      digitalWrite(firstLeftLine, HIGH);
      digitalWrite(secondLeftLine, HIGH);

      if (leftFlagOn) {
        digitalWrite(thirdLeftLine, HIGH);
      } else {
        digitalWrite(thirdLeftLine, LOW);
      }

      leftFlagOn = !leftFlagOn;
      delay(delayTime);
      break;
  }
}

void setRightLine() {
  switch (rightLine) {
    case 0:
      digitalWrite(firstRightLine, LOW);
      digitalWrite(secondRightLine, LOW);
      digitalWrite(thirdRightLine, LOW);
      break;
    case 1:
      digitalWrite(firstRightLine, HIGH);
      digitalWrite(secondRightLine, LOW);
      digitalWrite(thirdRightLine, LOW);
      break;
    case 2:
      digitalWrite(firstRightLine, HIGH);
      digitalWrite(secondRightLine, HIGH);
      digitalWrite(thirdRightLine, LOW);
      break;
    case 3:
      digitalWrite(firstRightLine, HIGH);
      digitalWrite(secondRightLine, HIGH);
      digitalWrite(thirdRightLine, HIGH);
      break;
    default:  // If more than 3 ships, third led blinks
      digitalWrite(firstRightLine, HIGH);
      digitalWrite(secondRightLine, HIGH);

      if (rightFlagOn) {
        digitalWrite(thirdRightLine, HIGH);
      } else {
        digitalWrite(thirdRightLine, LOW);
      }

      rightFlagOn = !rightFlagOn;
      delay(delayTime);
      break;
  }
}

void setChannel() {
  /*
   * The channel representation is given by percentages, so it's
   * necessary to implement for both left and right cases. 
   */
  turnOffChannel(); // Turn off all channel leds
  if (trafficLight == 1) {
    if (channel > 80) {
      digitalWrite(firstRightChannel, HIGH);
    } else if (channel > 60) {
      digitalWrite(secondRightChannel, HIGH);
    } else if (channel > 40) {
      digitalWrite(middleChannel, HIGH);
    } else if (channel > 20) {
      digitalWrite(secondLeftChannel, HIGH);
    } else if (channel > 1) {
      digitalWrite(firstLeftChannel, HIGH);
    }
  } else if (trafficLight == 2) {
    if (channel > 80) {
      digitalWrite(firstLeftChannel, HIGH);
    } else if (channel > 60) {
      digitalWrite(secondLeftChannel, HIGH);
    } else if (channel > 40) {
      digitalWrite(middleChannel, HIGH);
    } else if (channel > 20) {
      digitalWrite(secondRightChannel, HIGH);
    } else if (channel > 1) {
      digitalWrite(firstRightChannel, HIGH);
    }
  }
}

void loop() {

    if (Serial.available() > 0) { // New data received in serial port?
      delayTime = 0; // Turn off delay time to avoid communication lag
      if (data == 0 && counter == 0) {
        trafficLight = Serial.read();
        data++;
      } else if (data == 1 && counter == 0) {
        leftLine = Serial.read();
        data++;
      } else if (data == 2 && counter == 0) {
        rightLine = Serial.read();
        data++;
      } else if (data == 3 && counter == 0) {
        channel = Serial.read();
        data++;
      } else {
        incomingByte = Serial.read(); // Read and ignore trash data (0's)
      }

      counter++;

      if (counter > 3) {
        counter = 0;
      }
    } else {
      delayTime = delayTimeConstant; // Set delay time when not reading serial port
    }

    if (data > 3) {
      // Received data
      Serial.print("Traffic Light: ");
      Serial.println(trafficLight, DEC);
      Serial.print("Left Line: ");
      Serial.println(leftLine, DEC);
      Serial.print("Right Line: ");
      Serial.println(rightLine, DEC);
      Serial.print("Channel: ");
      Serial.println(channel, DEC);
      Serial.println();
      Serial.println();
      Serial.println();  
      data = 0;
    }  
    setTrafficLight();
    setLeftLine();
    setRightLine();
    setChannel();
}

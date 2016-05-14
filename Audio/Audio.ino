#include <LED.h>
#include <toneAC.h>

//consts for shift registers
const int DATA_PIN = 2;
const int LATCH_PIN = 3;
const int CLOCK_PIN = 4;

//consts for gameplay
const int SELECTOR_PIN = 5;
const int ACTION_PIN = 6;
const int P1_PIN = 7;
const int P2_PIN = 8;

//consts for mic
const int BUSY_LED_PIN = 13; // led connected to digital pin 13
const int ELECTRET = 0;  // the amplifier output is connected to analog pin 0

//global variables
int currentLED = 1;
int previousLED = 18;
bool selectorPressed = false;
bool actionPressed = false;
bool playerOneTurn = true;
bool playerOneWins = false;
bool playerTwoWins = false;
bool gameIsDraw = false;
bool screenSaverMode = true;
int sensorReading = 0;      // variable to store the value read from the sensor pin
int sensorMax = 0;
int sensorMin = 1023;
int threshold = 0;
int difference;
int startLvl = 200;

//set up shift registers
shiftOutX reg(3, 2, 4, MSBFIRST, 3);

//matrix for controlling LEDs
LED ledMatrix[3][3] = {LED(shPin1, shPin2), LED(shPin7, shPin8), LED(shPin13, shPin14),
                       LED(shPin3, shPin4), LED(shPin9, shPin10), LED(shPin15, shPin16),
                       LED(shPin5, shPin6), LED(shPin11, shPin12), LED(shPin17, shPin18)};
void setup() 
{
  //start with all LEDs off
  reg.allOff();

  //serial communication for debugging
  Serial.begin(9600);

  //set up pins for input or output
  pinMode(SELECTOR_PIN, INPUT);
  pinMode(ACTION_PIN, INPUT);
  pinMode(P1_PIN, OUTPUT);
  pinMode(P2_PIN, OUTPUT);
  pinMode(BUSY_LED_PIN, OUTPUT); // declare the ledPin as as OUTPUT

  //always start with the first LED selected
  ledMatrix[0][0].selected = true;

  //this is calibration for the mic
  //sets a baseline for the ambient noise in the room
  digitalWrite(BUSY_LED_PIN, HIGH);
  while (millis() < 3000) {
    threshold = analogRead(ELECTRET);

    // record the maximum sensor value
    if (threshold > sensorMax) {
      sensorMax = threshold;
    }

  }

  // signal the end of the calibration period
  digitalWrite(BUSY_LED_PIN, LOW);
  threshold = sensorMax;
}

//main loop
void loop() 
{
  //kind of like a main menu
  //this will run until the mic hears a loud sound like a clap
  while(screenSaverMode)
  {
    runScreenSaver();
  }

  //check the players turn
  whoseTurn();

  //checks for input and changes the currently selected LED
  currentSelectedLED();

  //checks for another input, whether the LED has been chosen by a player for their move
  actions();

  //updates LED states
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      ledMatrix[i][j].update(reg);
    }
  }

  //check for a win state with the LEDs (three in a row)
  checkForWin();

  //if a player has triggered a win state
  if(playerOneWins || playerTwoWins)
  {
    victory();
  }
}

//resets the game
void reset()
{
  currentLED = 1;
  previousLED = 18;
  selectorPressed = false;
  actionPressed = false;
  playerOneTurn = true;
  playerOneWins = false;
  playerTwoWins = false;
  gameIsDraw = false;
  screenSaverMode = true;

  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      ledMatrix[i][j].val = ' ';
      ledMatrix[i][j].selected = false;
    }
  }
}

//whose turn is it
//turns on and off an LED based on whose turn it is
void whoseTurn()
{
  if(playerOneTurn)
  {
    digitalWrite(P2_PIN, LOW);
    digitalWrite(P1_PIN, HIGH);
  }
  else
  {
    digitalWrite(P1_PIN, LOW);
    digitalWrite(P2_PIN, HIGH);
  }
}

//listens for a loud sound to act as a start button
void listenForStart()
{
  // read the sensor and store it in the variable sensorReading:
  sensorReading = analogRead(ELECTRET);    
  sensorMax = max(sensorMax, sensorReading);

  difference = 0;
  // if the sensor reading is greater than the threshold:
  if ((sensorReading >= threshold)) {
    difference = sensorReading-threshold;
  }
  
    Serial.println(difference);
  if(difference > startLvl)
  {   
    screenSaverMode = false;
  } 
}

void runScreenSaver()
{
  listenForStart();
}

//listens for player confirmation on which LED they chose
void actions()
{
  int input = digitalRead(ACTION_PIN);

  if(input == 1)
  {
    if(!actionPressed)
    {
      actionPressed = true;
      playConfirmTone();

      for(int i = 0; i < 3; i++)
      {
        for(int j = 0; j < 3; j++)
        {
          if(ledMatrix[i][j].selected && playerOneTurn)
          {
            ledMatrix[i][j].val = 'O';
            playerOneTurn = !playerOneTurn;
            return;
          }
          else if(ledMatrix[i][j].selected && !playerOneTurn)
          {
            ledMatrix[i][j].val = 'X';
            playerOneTurn = !playerOneTurn;
            return;
          }
        }
      }
    }
  }
  else actionPressed = false;
}

//plays a tone when the player changes LEDs
void playMoveTone()
{
  toneAC(300, 10, 180, true);
  delay(150);
  toneAC(600, 10, 230, true);
  delay(140);
}

//plays a tone when the player confirms an LED choice
void playConfirmTone()
{
  toneAC(600, 10, 230, true);
  delay(150);
  toneAC(300, 10, 180, true);
  delay(140);
}

//plays a tone and a small light show when a player has won
//then resets the game
void victory()
{
  if(playerTwoWins)
  {
    reg.pinOn(shPin1);
    reg.pinOn(shPin9);
    reg.pinOn(shPin17);
    toneAC(300, 10, 100, true);
    delay(150);
    reg.pinOn(shPin3);
    reg.pinOn(shPin15);
    toneAC(300, 10, 100, true);
    delay(150);
    reg.pinOn(shPin5);
    reg.pinOn(shPin13);
    toneAC(300, 10, 100, true);
    delay(150);
    reg.pinOn(shPin7);
    reg.pinOn(shPin11);
    reg.pinOn(shPin17);
    toneAC(700, 10, 428, true);
    delay(1000);
    reg.allOff();
  }
  else if(playerOneWins)
  {
    reg.pinOn(shPin2);
    reg.pinOn(shPin10);
    reg.pinOn(shPin18);
    toneAC(300, 10, 100, true);
    delay(150);
    reg.pinOn(shPin4);
    reg.pinOn(shPin16);
    toneAC(300, 10, 100, true);
    delay(150);
    reg.pinOn(shPin6);
    reg.pinOn(shPin14);
    toneAC(300, 10, 100, true);
    delay(150);
    reg.pinOn(shPin8);
    reg.pinOn(shPin12);
    reg.pinOn(shPin18);
    toneAC(700, 10, 428, true);
    delay(1000);
    reg.allOff();
  }

  reset();
}

// figure out which led is currently selected
// based on button input
void currentSelectedLED()
{
  int input = digitalRead(SELECTOR_PIN);

  if(input == 1)
  {
    if(!selectorPressed)
    {
      selectorPressed = true;
      previousLED = currentLED;
      currentLED++;
      playMoveTone();
    }
  }
  else selectorPressed = false;

  if(currentLED > 9)
  {
    currentLED = 1;
  }

  switch(currentLED)
  {
    case 1:
    {
      ledMatrix[2][2].selected = false;
      if(ledMatrix[0][0].val == 'X' || ledMatrix[0][0].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[0][0].selected = true;
      break;
    }
    case 2:
    {
      ledMatrix[0][0].selected = false;
      if(ledMatrix[1][0].val == 'X' || ledMatrix[1][0].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[1][0].selected = true;
      break;
    }
    case 3:
    {
      ledMatrix[1][0].selected = false;
      if(ledMatrix[2][0].val == 'X' || ledMatrix[2][0].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[2][0].selected = true;
      break;
    }
    case 4:
    {
      ledMatrix[2][0].selected = false;
      if(ledMatrix[0][1].val == 'X' || ledMatrix[0][1].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[0][1].selected = true;
      break;
    }
    case 5:
    {
      ledMatrix[0][1].selected = false;
      if(ledMatrix[1][1].val == 'X' || ledMatrix[1][1].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[1][1].selected = true;
      break;
    }
    case 6:
    {
      ledMatrix[1][1].selected = false;
      if(ledMatrix[2][1].val == 'X' || ledMatrix[2][1].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[2][1].selected = true;
      break;
    }
    case 7:
    {
      ledMatrix[2][1].selected = false;
      if(ledMatrix[0][2].val == 'X' || ledMatrix[0][2].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[0][2].selected = true;
      break;
    }
    case 8:
    {
      ledMatrix[0][2].selected = false;
      if(ledMatrix[1][2].val == 'X' || ledMatrix[1][2].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[1][2].selected = true;
      break;
    }
    case 9:
    {
      ledMatrix[1][2].selected = false;
      if(ledMatrix[2][2].val == 'X' || ledMatrix[2][2].val == 'O')
      {
        currentLED++;
        break;
      }
      ledMatrix[2][2].selected = true;
    }
  }
}

// check for a win state
void checkForWin()
{
  //check 3 horizontal conditions
  //then 3 vertical conditions
  //then 2 diaganal conditions
  //check this for player 1 and player 2
  //total of 16 win conditions
  if(//row 1 collunm 1                row 1 collunm 2               row 1 collunm 3
     ledMatrix[0][0].val == 'O' && ledMatrix[0][1].val == 'O' && ledMatrix[0][2].val == 'O' ||
     //row 2 collunm 1                row 2 collunm 2               row 2 collunm 3
     ledMatrix[1][0].val == 'O' && ledMatrix[1][1].val == 'O' && ledMatrix[1][2].val == 'O' ||
     //row 3 collunm 1                row 3 collunm 2               row 3 collunm 3
     ledMatrix[2][0].val == 'O' && ledMatrix[2][1].val == 'O' && ledMatrix[2][2].val == 'O' ||

     //row 1 collunm 1                row 2 collunm 1               row 3 collunm 1
     ledMatrix[0][0].val == 'O' && ledMatrix[1][0].val == 'O' && ledMatrix[2][0].val == 'O' ||
     //row 1 collunm 2                row 2 collunm 2               row 3 collunm 2
     ledMatrix[0][1].val == 'O' && ledMatrix[1][1].val == 'O' && ledMatrix[2][1].val == 'O' ||
     //row 1 collunm 3                row 2 collunm 3               row 3 collunm 3
     ledMatrix[0][2].val == 'O' && ledMatrix[1][2].val == 'O' && ledMatrix[2][2].val == 'O' ||
     
     //row 1 collunm 1                row 2 collunm 2               row 3 collunm 3
     ledMatrix[0][0].val == 'O' && ledMatrix[1][1].val == 'O' && ledMatrix[2][2].val == 'O' ||
     //row 1 collunm 3                row 2 collunm 2               row 3 collunm 1
     ledMatrix[0][2].val == 'O' && ledMatrix[1][1].val == 'O' && ledMatrix[2][0].val == 'O')
  {
    playerOneWins = true;
  }
  else if(//row 1 collunm 1                row 1 collunm 2               row 1 collunm 3
          ledMatrix[0][0].val == 'X' && ledMatrix[0][1].val == 'X' && ledMatrix[0][2].val == 'X' ||
          //row 2 collunm 1                row 2 collunm 2               row 2 collunm 3
          ledMatrix[1][0].val == 'X' && ledMatrix[1][1].val == 'X' && ledMatrix[1][2].val == 'X' ||
          //row 3 collunm 1                row 3 collunm 2               row 3 collunm 3
          ledMatrix[2][0].val == 'X' && ledMatrix[2][1].val == 'X' && ledMatrix[2][2].val == 'X' ||
    
          //row 1 collunm 1                row 2 collunm 1               row 3 collunm 1
          ledMatrix[0][0].val == 'X' && ledMatrix[1][0].val == 'X' && ledMatrix[2][0].val == 'X' ||
          //row 1 collunm 2                row 2 collunm 2               row 3 collunm 2
          ledMatrix[0][1].val == 'X' && ledMatrix[1][1].val == 'X' && ledMatrix[2][1].val == 'X' ||
          //row 1 collunm 3                row 2 collunm 3               row 3 collunm 3
          ledMatrix[0][2].val == 'X' && ledMatrix[1][2].val == 'X' && ledMatrix[2][2].val == 'X' ||
         
          //row 1 collunm 1                row 2 collunm 2               row 3 collunm 3
          ledMatrix[0][0].val == 'X' && ledMatrix[1][1].val == 'X' && ledMatrix[2][2].val == 'X' ||
          //row 1 collunm 3                row 2 collunm 2               row 3 collunm 1
          ledMatrix[0][2].val == 'X' && ledMatrix[1][1].val == 'X' && ledMatrix[2][0].val == 'X')
  {
    playerTwoWins = true;
  }
  //was trying to figure out a draw condition, but I ran out of time to work on it
  /*else if(//row 1 collunm 1
          ledMatrix[0][0].val == 'X' || ledMatrix[0][0].val == 'O' &&
          //row 1 collunm 2
          ledMatrix[0][1].val == 'X' || ledMatrix[0][1].val == 'O' &&
          //row 1 collunm 3
          ledMatrix[0][2].val == 'X' || ledMatrix[0][2].val == 'O' &&
          //row 2 collunm 1
          ledMatrix[1][0].val == 'X' || ledMatrix[1][0].val == 'O' &&
          //row 2 collunm 2
          ledMatrix[1][1].val == 'X' || ledMatrix[1][1].val == 'O' &&
          //row 2 collunm 3
          ledMatrix[1][2].val == 'X' || ledMatrix[1][2].val == 'O' &&
          //row 3 collunm 1
          ledMatrix[2][0].val == 'X' || ledMatrix[2][0].val == 'O' &&
          //row 3 collunm 2
          ledMatrix[2][1].val == 'X' || ledMatrix[2][1].val == 'O' &&
          //row 3 collunm 3
          ledMatrix[2][2].val == 'X' || ledMatrix[2][2].val == 'O')
  {
    gameIsDraw = true;
    Serial.println("draw");
    Serial.println(ledMatrix[0][1].val);
  }*/
}


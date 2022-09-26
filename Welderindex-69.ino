#include <Arduino.h>
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3f,20,4); 
//150 --> 0-30seg
//300 --> 30-60seg
//450 --> 60-90seg
#define StepsPerRevolution 200  // Change this to represent the number of steps
                                //   it takes the motor to do one full revolution
                                //   200 is the result of a 1.8 degree per step motor
#define Microsteps 8            // Depending on your stepper driver, it may support
                                //   microstepping.  Set this number to the number of microsteps
                                //   that is set on the driver board.  For large ratios, you may want to 
                                //   use no microstepping and set this to 1.
#define GearRatio1top  1        // Change these three values to reflect any front end gearing you 
#define GearRatio1bottom 1      //   this is the bottom of the ratio - usually 1, as in 3 to 1, but you may change that here.
#define GearRatio2top  2       //40   are using for three different devices.  If no gearing, then
#define GearRatio2bottom 1      //   40 to 1
#define GearRatio3top  3       //90   define this value to be 1.  GearRatio1 is the default
#define GearRatio3bottom 1      //   90 to 1

#define GearRatioMax 3          // number of above gear ratios defined

#define AngleIncrement 5        // Set how much a keypress changes the angle setting

#define CW HIGH                 // Define direction of rotation - 
#define CCW LOW                 // If rotation needs to be reversed, swap HIGH and LOW here

#define DefaultMotorSpeed 0     // zero here means fast, as in no delay, estaba en 150

// define menu screen ids


#define mainmenu 0
#define runmode 1 //Pulsado
#define stepmode 2 //run 30/60
#define anglemode 3 //run 60/90
#define jogmode 4
#define ratiomode 5 //cambio radio, default 1:1
#define numModes  5 




#define moveangle 10
#define movesteps 11

//#define Celsius 1         // define only one of these, please!
//#define Fahrenheit 1        // Fahrenheit is default

// Define the pins that the driver is attached to.
// Once set, this are normally not changed since they correspond to the wiring.

#define motorSTEPpin   2    // output signal to step the motor
#define motorDIRpin    3    // output siagnal to set direction
#define motorENABLEpin 11   // output pin to power up the motor
#define AnalogKeyPin   0    // keypad uses A0
#define SinkTempPin    1    // temp sensor for heatsink is pin A1
#define MotorTempPin   2    // temp sensor for motor is pin A2

#define pulsewidth     2    // length of time for one step pulse
#define ScreenPause    800  // pause after screen completion
#define ADSettleTime   10   // ms delay to let AD converter "settle" i.e., discharge

#define SpeedDelayIncrement 50  // change value of motor speed steps (delay amount) era 5
#define JogStepsIncrement 1    // initial value of number of steps per keypress in test mode
#define TSampleSize 7          // size of temperature sampling to average

// create lcd display construct
//Crear el objeto LCD con los números correspondientes (rs, en, d4, d5, d6, d7)
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  //Data Structure and Pin Numbers for the LCD/Keypad

// define LCD/Keypad buttons
#define NO_KEY 0
#define SELECT_KEY 1
#define LEFT_KEY 2
#define UP_KEY 3
#define DOWN_KEY 4
#define RIGHT_KEY 5

// create global variables
int pote,customDelayMapped;   // lectura del potenciometro
int customDelay;
int newCustom;
int grafico;

int    cur_mode = mainmenu;
int    mode_select = stepmode;
int    current_menu;
float  cur_angle = 0;
int    cur_key;
int    num_divisions = 0;
int    numjogsteps = JogStepsIncrement;
unsigned long stepsperdiv;
int    cur_pos = 0;
int    cur_dir = CW;
int    motorspeeddelay = DefaultMotorSpeed;
unsigned long motorSteps;
unsigned long gear_ratio_top_array[GearRatioMax] = {GearRatio1top,GearRatio2top,GearRatio3top};
unsigned long gear_ratio_bottom_array[GearRatioMax] = {GearRatio1bottom,GearRatio2bottom,GearRatio3bottom};
int    gearratioindex = 0;      // the first array element starts with 0 and is the default ration chosen

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.begin(20, 4);
  lcd.clear();

  
//  Create some custom characters for the lcd display
  byte c_CW[8] =       {0b01101,0b10011,0b10111,0b10000,0b10000,0b10000,0b10001,0b01110}; //Clockwise
  byte c_CCW[8] =      {0b10110,0b11001,0b11101,0b00001,0b00001,0b00001,0b10001,0b01110}; // CounterClockWise
  byte c_UPP[8] =       {0b00100,0b01110,0b11111,0b01100,0b01100,0b01100,0b00000,0b00000}; //arrow
  byte c_DOP[8] =       {0b00000,0b00000,0b01100,0b01100,0b01100,0b11111,0b01110,0b00100}; //arrow  
  lcd.createChar(1,c_CW);
  lcd.createChar(2,c_CCW);
  lcd.createChar(3,c_UPP);
  lcd.createChar(4,c_DOP);


  
  // begin program
  
  motorSteps = (gear_ratio_top_array[gearratioindex]*Microsteps*StepsPerRevolution)/gear_ratio_bottom_array[gearratioindex];
  lcd.begin(20, 4);
  lcd.clear();
  lcd.setCursor(0,0);                 // display flash screen
  lcd.print("DimmersJujuy   v0.96");
  lcd.setCursor(0,1);                 // display flash screen
  lcd.print("Soldador Rotativo");
  lcd.setCursor(0,2);
  lcd.print("Radio =");
  lcd.setCursor(10,2);
  lcd.print(gear_ratio_top_array[gearratioindex]);
  lcd.setCursor(12,2);
  lcd.print(":");
  lcd.print(gear_ratio_bottom_array[gearratioindex]);
  delay(3069);                        // wait a few secs
  lcd.clear();
  
  pinMode(motorSTEPpin, OUTPUT);     // set pin 3 to output
  pinMode(motorDIRpin, OUTPUT);      // set pin 2 to output
  pinMode(motorENABLEpin, OUTPUT);   // set pin 11 to output
  digitalWrite(motorENABLEpin,HIGH); // power up the motor and leave it on

  displayscreen(cur_mode);           // put up initial menu screen

}                                    // end of setup function

void loop()                          // main loop services keystrokes
{ 
  int exitflag;
  int numjogsteps;
  displayscreen(cur_mode);            // display the screen
  cur_key = get_real_key();           // grab a keypress
  switch(cur_mode)                    // execute the keystroke
  {
    case mainmenu:                    // main menu
      switch(cur_key) 
      {
        case UP_KEY:
          mode_select++;
          if (mode_select > numModes)  // wrap around menu
            mode_select = 1;
          break;
        case DOWN_KEY:
          mode_select--;
          if (mode_select < 1)         // wrap around menu
            mode_select = numModes;
          break;
        case LEFT_KEY:           // left and right keys do nothing in main menu
          break;
        case RIGHT_KEY:
          break;
        case SELECT_KEY:         // user has picked a menu item
          cur_mode = mode_select;
          break;
      } 
      break;
    case runmode:                     // call run  
      dorunmode(cur_key);
      cur_mode = mainmenu;
      break; 
    case stepmode:                    // call steps
      dostepmode(cur_key);
      cur_mode = mainmenu;
      break;
    case anglemode:                   // call angles
      doanglemode(cur_key);
      cur_mode = mainmenu;
      break;
    case jogmode:                    // call jog
      dojogmode(cur_key);
      cur_mode = mainmenu;
      break;
    case ratiomode:                 // call ratio
      doratiomode(cur_key);
      cur_mode = mainmenu;
      break;  
    
  }                                           // end of mode switch
}                                               // end of main loop

void dostepmode(int tmp_key)
{
  int breakflag = 0;
  delay(100);                              // wait for keybounce from user's selection
  cur_dir = CW;                            // initially, clockwise
  displayscreen(runmode);                  // show the screen
  while (breakflag == 0)                   // cycle until Select Key sets flag
  {     
        customDelay = analogRead(1); // Reads the potentiometer
        newCustom = map(customDelay, 0, 1023, 150,300); // Convrests the read values of the potentiometer from 0 to 1023 into desireded delay values (300 to 4000)
        motorspeeddelay = newCustom;
        lcd.setCursor(13,1);//
        lcd.print(newCustom);//
          
    move_motor(1,cur_dir,0);               // move motor 1 step
    
    if (analogRead(AnalogKeyPin) < 850 )  // if a keypress is present       {
    {                          
      cur_key = get_real_key();            // then get it
      switch(cur_key)                       // and honor it
      {

      case LEFT_KEY:                      // set direction
        cur_dir = CCW;
        motorspeeddelay = newCustom;
        displayscreen(runmode); 
        break;
      case RIGHT_KEY:                     // set other direction
        cur_dir = CW;
        motorspeeddelay = newCustom;
        displayscreen(runmode);      
        break;
        case UP_KEY:           // left and right keys do nothing in main menu
          break;
        case DOWN_KEY:
          break;
      case SELECT_KEY:                   // user wants to stop
        motorspeeddelay = DefaultMotorSpeed;   // reset speed   
        breakflag = 1;                   // fall through to exit
      }
    }  
  }     
} 
void doanglemode(int tmp_key)
{
  int breakflag = 0;
  delay(100);                              // wait for keybounce from user's selection
  cur_dir = CW;                            // initially, clockwise
  displayscreen(runmode);                  // show the screen
  while (breakflag == 0)                   // cycle until Select Key sets flag
  {     
        customDelay = analogRead(1); // Reads the potentiometer
        newCustom = map(customDelay, 0, 1023, 295,453); // Convrests the read values of the potentiometer from 0 to 1023 into desireded delay values (300 to 4000)
        motorspeeddelay = newCustom;
        lcd.setCursor(13,1);//
        lcd.print(newCustom);//
          
    move_motor(1,cur_dir,0);               // move motor 1 step
    
    if (analogRead(AnalogKeyPin) < 850 )  // if a keypress is present       {
    {                          
      cur_key = get_real_key();            // then get it
      switch(cur_key)                       // and honor it
      {

      case LEFT_KEY:                      // set direction
        cur_dir = CCW;
        motorspeeddelay = newCustom;
        displayscreen(runmode); 
        break;
      case RIGHT_KEY:                     // set other direction
        cur_dir = CW;
        motorspeeddelay = newCustom;
        displayscreen(runmode);      
        break;
      case SELECT_KEY:                   // user wants to stop
        motorspeeddelay = DefaultMotorSpeed;   // reset speed   
        breakflag = 1;                   // fall through to exit
      }
    }  
  }     
} 
void dorunmode(int tmp_key)
{
  int valor_leido = 0;
  int breakflag = 0;
  delay(100);                              // wait for keybounce from user's selection
  cur_dir = CW;   
  displayscreen(runmode);                  // show the screen
  while (breakflag == 0)                   // cycle until Select Key sets flag
  {     
        customDelay = analogRead(1); // Reads the potentiometer
        newCustom = map(customDelay, 0, 1023, 1,150); // Convrests the read values of the potentiometer from 0 to 1023 into desireded delay values (300 to 4000)
        motorspeeddelay = newCustom;
        lcd.setCursor(13,1);//
        lcd.print(newCustom);//
        lcd.print("   "); 
        move_motor(1,cur_dir,0);               // move motor 1 step
        
    if (analogRead(AnalogKeyPin) < 850 )  // if a keypress is present       {
    {                          
      cur_key = get_real_key();            // then get it
      switch(cur_key)                       // and honor it
      {  
      case LEFT_KEY:                      // set direction
        cur_dir = CCW;
        motorspeeddelay = newCustom;
        displayscreen(runmode); 
      
        break;
      case RIGHT_KEY:                     // set other direction
        cur_dir = CW;
        motorspeeddelay = newCustom;
        displayscreen(runmode); 
      case UP_KEY:           // left and right keys do nothing in main menu
        break;
      case DOWN_KEY:
        break;
        break;
      case SELECT_KEY:                   // user wants to stop
        motorspeeddelay = DefaultMotorSpeed;   // reset speed   
        breakflag = 1;                   // fall through to exit
      }
    }  
  }     
}  

void doratiomode(int tmp_key)  //CAMBIO DE RATIO DE ENGRANAJES
{
  int breakflag = 0;
  
  displayscreen(ratiomode); 
  for (;breakflag==0;)
  {
    switch(tmp_key) 
    {
      case UP_KEY:                          // bump the number of steps
          ++gearratioindex;
          if (gearratioindex > GearRatioMax-1)  //wrap around if over the max
            gearratioindex = 0;
          motorSteps = (gear_ratio_top_array[gearratioindex]*Microsteps*StepsPerRevolution)/gear_ratio_bottom_array[gearratioindex];
          break;
      case DOWN_KEY:                        // reduce the number of steps
          --gearratioindex;
          if (gearratioindex < 0)
            gearratioindex = GearRatioMax-1;    // wrap around if already at the bottom  
          motorSteps = (gear_ratio_top_array[gearratioindex]*Microsteps*StepsPerRevolution)/gear_ratio_bottom_array[gearratioindex];
          break;
      case LEFT_KEY:                        //  Left and Right keys do nothing in this mode 
          break;
      case RIGHT_KEY:                       
          break;
      case SELECT_KEY:                      // user want to quit
          breakflag = 1;
          break;
    }
    if (breakflag == 0)
    {
      displayscreen(ratiomode);
      tmp_key = get_real_key();
    }  
  }  
  numjogsteps = JogStepsIncrement;
  cur_mode = mainmenu;                 // go back to main menu
  return; 
}
void dojogmode(int tmp_key)//TIG
{
  int valor_leido = 0;
  int breakflag = 0;
  delay(100);                              // wait for keybounce from user's selection
  cur_dir = CW;   
  displayscreen(runmode);                  // show the screen
  while (breakflag == 0)                   // cycle until Select Key sets flag
  {     
        customDelay = analogRead(1); // Reads the potentiometer
        lcd.setCursor(13,3);
        lcd.print(motorSteps);
        newCustom = map(customDelay, 0, 1023, 1,450); // Convrests the read values of the potentiometer from 0 to 1023 into desireded delay values (300 to 4000)
        motorspeeddelay = newCustom;
        lcd.setCursor(13,1);//
        lcd.print(newCustom);//
        lcd.print("   "); 
        move_motor2(26,cur_dir,6);               // move motor 3 step y pausa
        
    if (analogRead(AnalogKeyPin) < 850 )  // if a keypress is present       {
    {                          
      cur_key = get_real_key();            // then get it
      switch(cur_key)                       // and honor it
      {  
      case LEFT_KEY:                      // set direction
        cur_dir = CCW;
        motorspeeddelay = newCustom;
        displayscreen(runmode); 
      
        break;
      case RIGHT_KEY:                     // set other direction
        cur_dir = CW;
        motorspeeddelay = newCustom;
        displayscreen(runmode); 
           
        break;
      case SELECT_KEY:                   // user wants to stop
        motorspeeddelay = DefaultMotorSpeed;   // reset speed   
        breakflag = 1;                   // fall through to exit
      }
    }  
  }     
}  

void displayscreen(int menunum)        // screen displays are here
{
 lcd.clear(); 
 lcd.setCursor(0,0);
 switch (menunum) 
 {
  case mainmenu:
    lcd.print("Selecione Modo");
    lcd.setCursor(18,0);
    lcd.write(byte(3));
    lcd.setCursor(19,0);
    lcd.write(byte(4));
    
    lcd.setCursor(0,1);
    lcd.print("Modo = "); 
 
    lcd.setCursor(7,1);
    switch(mode_select) 
    {
      case(runmode):
        lcd.print("Run Rev./30s");//150
        break;  
      case(stepmode):
        lcd.print("Run Rev./60s");//300
        break;
      case(anglemode):
        lcd.print("Run Rev./90s");//450
        break;
      case(jogmode):
        lcd.print("TIG x Puntos");
        break; 
      case(ratiomode):
        lcd.print("Radio");
        break;      
    }  
    break;
  case runmode://30
    lcd.print("Continuo");
    lcd.setCursor(11,0);
    if (cur_dir == CW)
      lcd.write(byte(1));
    else
      lcd.write(byte(2)); 
    lcd.setCursor(0,1);
    lcd.print("Velocidad =");
    lcd.setCursor(13,1);
    lcd.print((int)motorspeeddelay);//
    break;
      case stepmode://60
    lcd.print("Continuo");
    lcd.setCursor(11,0);
    if (cur_dir == CW)
      lcd.write(byte(1));
    else
      lcd.write(byte(2)); 
    lcd.setCursor(0,1);
    lcd.print("Velocidad =");
    lcd.setCursor(13,1);
    lcd.print((int)motorspeeddelay);
    break;
  case anglemode://90
    lcd.print("Continuo");
    lcd.setCursor(11,0);
    if (cur_dir == CW)
      lcd.write(byte(1));
    else
      lcd.write(byte(2)); 
    lcd.setCursor(0,1);
    lcd.print("Velocidad =");
    lcd.setCursor(13,1);
    lcd.print((int)motorspeeddelay);
    break;
  case jogmode://punto
    lcd.print("Intermitente");
    lcd.setCursor(11,0);
    if (cur_dir == CW)
      lcd.write(byte(1));
    else
      lcd.write(byte(2)); 
    lcd.setCursor(0,1);
    lcd.print("Velocidad =");
    lcd.setCursor(13,1);
    lcd.print((int)motorspeeddelay);//
    break; 
  case ratiomode://radio
    lcd.setCursor(0,0);
    lcd.print("Radio =");
    lcd.setCursor(4,1);
    lcd.print(gear_ratio_top_array[gearratioindex]);
    lcd.setCursor(10,1);
    lcd.print(":");
    lcd.setCursor(11,1);
    lcd.print(gear_ratio_bottom_array[gearratioindex]);
    break;  

  }
  return;
}  

void move_motor(unsigned long steps, int dir, int type)
{
  unsigned long i;
  //        move_motor(1,cur_dir,0);               // move motor 1 step
  if (type == movesteps)
    displayscreen(movesteps);
  if (type == moveangle)
    displayscreen(moveangle);    
  for (i=0;i<steps;i++)
  {
    digitalWrite(motorDIRpin,dir);
    digitalWrite(motorSTEPpin,HIGH);   //pulse the motor
    delay(pulsewidth);
    digitalWrite(motorSTEPpin,LOW);
    if (type == movesteps)             // in this mode display progress
    {
      lcd.setCursor(10,1);
      lcd.print(i);
    }
    if (type == moveangle)             // in this mode display progress
    {
      lcd.setCursor(7,0);
      lcd.print(i);
    }   
    delay(motorspeeddelay);            // wait betweeen pulses
  }  
  return;
}  
void move_motor2(unsigned long steps, int dir, int type)
{
  unsigned long i;
  //        move_motor(1,cur_dir,0);               // move motor 1 step
  if (type == movesteps)
    displayscreen(movesteps);
  if (type == moveangle)
    displayscreen(moveangle);    
  for (i=0;i<steps;i++)
  {
    digitalWrite(motorDIRpin,dir);
    digitalWrite(motorSTEPpin,HIGH);   //pulse the motor
    delay(pulsewidth);
    digitalWrite(motorSTEPpin,LOW);
    if (type == movesteps)             // in this mode display progress
    {
      lcd.setCursor(10,1);
      lcd.print(i);
    }
    if (type == moveangle)             // in this mode display progress
    {
      lcd.setCursor(7,0);
      lcd.print(i);
    }   
    delay(motorspeeddelay);            // wait betweeen pulses
  }  
  delay(1000);
  return;
}  

int get_real_key(void)    // routine to return a valid keystroke
{
  int trial_key = 0;
  while (trial_key < 1)
  {
    trial_key = read_LCD_button(); 
  }
  delay(200);             // 200 millisec delay between user keys
  return trial_key;
}  

int read_LCD_button()     // routine to read the LCD's buttons
{
  int key_in;
  delay(ADSettleTime);         // wait to settle
  key_in = analogRead(0);      // read ADC once
  delay(ADSettleTime);         // wait to settle
  // average values for my board were: 0, 144, 324, 505, 742
  // add approx 100 to those values to set range
//lcd.clear();
 //lcd.print(key_in);
  //delay(800); test
  // lcd.setCursor(0,2);
 //   lcd.print(key_in);
 
  // keypad final
  if (key_in > 1000) return NO_KEY;    
  if (key_in < 50)   return RIGHT_KEY;   //13
  if (key_in < 200)  return UP_KEY; //154
  if (key_in < 450)  return DOWN_KEY; //346
  if (key_in < 700)  return LEFT_KEY; //593
  if (key_in < 850)  return SELECT_KEY; //760
  
} 

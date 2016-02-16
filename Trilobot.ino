/* Trilobot Rex 
// interrupt demo
// calls specified function after specified number of 50us ticks
//
 MGP July 2015
 */

#include <Trilobot.h>



LED tickLED; 
unsigned long ticksNow;

// Trilobot class instance
Trilobot bot;  

float x[2*_N_MAGCAL_DATA][2];

void setup()
{
  bot.init();                      // initialize bot
}

void loop()
{
  
   int i;

// {
//  // flash LEDs randomly


     
     
//  int thisLED = random(4); // pick one of the 6 LEDs at random
//  bot.LED(thisLED, ON);
//  delay(25);
//  bot.LED(thisLED, OFF);
//}


//  
//  //bot.LEDs(FrontRed);
//
//  // side LEDs on when within D deg of 90,180 or 270
//  float D = 5.;
//  bearing = compass();
//  bot.LED(LEFT_GREEN,OFF);
//  bot.LED(LEFT_GREEN,OFF);
//  bot.LED(RIGHT_GREEN,OFF);
//  if (abs(bearing-90)<D | abs(bearing)> 180-D ) bot.LED(LEFT_GREEN,ON);  
//  if (abs(bearing+90)<D | abs(bearing)> 180-D ) bot.LED(RIGHT_GREEN,ON);  
  
//  if ((bot.ticks/2)*2==bot.ticks) bot.LED(tickLED, ON);
//  else bot.LED(tickLED, OFF);


  
  if (bot.ticked) {
    
    ticksNow = bot.ticks;
    
    switch (bot.mode) {
   
      case LINE_FOLLOWER:
      
       // bot.forward(120);
        
        int Leye = bot.readLightsensor(_LEFT);
        int Reye = bot.readLightsensor(_RIGHT);
        int sign = Reye>Leye? 1:-1;
    
//        analogWrite(_LEFT_MOTOR_SPEED_PIN,  50 - sign*(25*Leye)/(Leye+Reye));
//        analogWrite(_RIGHT_MOTOR_SPEED_PIN, 50 + sign*(25*Reye)/(Leye+Reye));

        analogWrite(_LEFT_MOTOR_SPEED_PIN,  50 - sign*50);
        analogWrite(_RIGHT_MOTOR_SPEED_PIN, 50 + sign*50);
       
        
         Serial.print(Leye);
         Serial.print(", ");
         Serial.println(Reye);
         break;
      
   
    }   
 
   // light show
   switch (bot.buttonContext) {
     
     case Startup_button_context:  // random flash
//       bot.LEDoff();  // all LEDs off
//       bot.LED(random(6), ON); // random LED on
//       bot.LED(random(6), ON); // random LED on
//       bot.LED(random(6), ON); // random LED on
       bot.mode = REMOTE_CONTROL;
       
       break;
       
     case A_button_context: // anticlockwise cycle
       bot.LEDoff();
       bot.LED(bot.ticks % 6, ON);
       break;
       
     case B_button_context: // clockwise cycle
       bot.LEDoff();
       bot.LED(5 - bot.ticks % 6, ON);
       break;   
   
     case C_button_context: // compass 
       // front LED fades from red to green within 5deg of heading 0
       bot.LEDoff();
       float bearing = bot.compass();
       int redness = 25.*(1.-cos(bot.pi*bearing/180.));
       bot.LED_colour(_FRONT_LED_LIGHT, redness, (abs(bearing)<2)? 255:0);  
     
       if (bearing<4) bot.LED(RIGHT_GREEN, ON);
         else if (bearing<8) bot.LED(RIGHT_RED, ON);
       if (bearing>-4) bot.LED(LEFT_GREEN, ON);
         else if (bearing>-8) bot.LED(LEFT_RED, ON);

     
       
       break;
   }
    
    switch(bot.getSparkfunRemoteButtonPress()) {
      
    // Power button: 
    // Stop, -> startup mode
    case POWER_buttonpressed:
      bot.buttonContext = Startup_button_context;
      bot.forward(0);
      bot.LED(FRONT_RED, ON); 
      delay(50);
      bot.LED(FRONT_RED, OFF);  
      delay(50);
     break;
      
    // A button
    // 
    case A_buttonpressed:
      bot.buttonContext = A_button_context;
      bot.forward(0);
      for (int i = 0; i<2; i++) {
        bot.LED(FRONT_RED, ON); 
        delay(100);
        bot.LED(FRONT_RED, OFF);  
        delay(100);
      }
      break;  
      
    // B button
    // 
    case B_buttonpressed:
      bot.buttonContext = B_button_context;
     // bot.forward(0);
      for (int i = 0; i<3; i++) {
        bot.LED(FRONT_RED, ON); 
        delay(100);
        bot.LED(FRONT_RED, OFF);  
        delay(100);
      }      break;     
      
    // C button
    // calibration mode
    case C_buttonpressed:
      bot.buttonContext = C_button_context;
      bot.forward(0);
      for (int i = 0; i<4; i++) {
        bot.LED(FRONT_RED, ON); 
        delay(100);
        bot.LED(FRONT_RED, OFF);  
        delay(100);
      }      
      break; 
   
      
    // O (centre) button  
    case O_buttonpressed:
       switch (bot.buttonContext){
         case C_button_context:
           compass_calibrate(200);
          break; 
         
       }
      break;
      
      
    case UP_buttonpressed:
      switch (bot.buttonContext){
        
        case Startup_button_context:
          bot.accelerate(_DEFAULT_ACCELERATION);
          break;
          
        case B_button_context:
          bot.waggle(2,8);
          break;
          
          
          
      }
      break;
      
    case DOWN_buttonpressed:
       switch (bot.buttonContext){
        
        case Startup_button_context:
          bot.accelerate(-_DEFAULT_ACCELERATION);
          break;
          
      }
      break;
      
    case LEFT_buttonpressed:
       switch (bot.buttonContext){
        
        case Startup_button_context:
         bot.tickTurn(_LEFT, 2);
         //delay(100);
          break;
          
       case B_button_context: // line follower
       
         bot.mode = LINE_FOLLOWER;
         break;
          
      }
      break;    
      
    case RIGHT_buttonpressed:
       switch (bot.buttonContext){
        
        case Startup_button_context:
          bot.tickTurn(_RIGHT,2);
          //delay(100);
          break;
          
      }
      break;  
  
       
    }
    // disable this block until next tick
    bot.ticked = false;

  }
  
  // enable tick operations on next tick
  else if (bot.ticks > ticksNow) bot.ticked = true;

}


void compass_calibrate(int sampleInterval){
  // Calibrate magnetometer.
  // Bot circles one way then the other, taking readings.
  // Fit an ellipsoid to get calibration parameters.
  // Empirically the data lie almost on a circle unless there is a soft iron
  // perturbation, in which case the compass will be uselss anyway (without a 
  // lot of extra work)so we assume a circle fit.
  //
  // Number of samples and default sample interval in ms are defined in Trilobot.h
  // Default is used if sampleInterval<1
  // User should check that bot does at least 1 full turn in each direction.
  // Change samplePeriod to fix this.
  // 
  
  if (sampleInterval<1)sampleInterval = _SAMPLE_DT_MAGCAL;

  int i;
  
  float x_bar = 0;
  float y_bar = 0;

  // spin left, take x,y magentometer data
  // and compute mean recursively
  bot.motors(_MAGCAL_SPEED,0);
  for(i=0; i<_N_MAGCAL_DATA; i++) {
   // bot.magnetSense();//*********************************************************
    delay(sampleInterval);
    x[i][0] = float(bot.state.mx);
    x_bar = float(i)/float(i+1)*x_bar + x[i][0]/float(i+1);
    x[i][1] = float(bot.state.my);
    y_bar = float(i)/float(i+1)*y_bar + x[i][1]/float(i+1);
  }

  // spin right, take data
  bot.motors(0, _MAGCAL_SPEED);
  for(i=0; i<_N_MAGCAL_DATA; i++) {
 //   bot.magnetSense();  //************************************************
    delay(sampleInterval);
    x[_N_MAGCAL_DATA+i][0] = float(bot.state.mx);
    x_bar = float(_N_MAGCAL_DATA+i)/float(_N_MAGCAL_DATA+i+1)*x_bar + 
            x[_N_MAGCAL_DATA+i][0]/float(_N_MAGCAL_DATA+i+1);
    x[_N_MAGCAL_DATA+i][1] = float(bot.state.my);
    y_bar = float(_N_MAGCAL_DATA+i)/float(_N_MAGCAL_DATA+i+1)*y_bar + 
            x[_N_MAGCAL_DATA+i][1]/float(_N_MAGCAL_DATA+i+1);    
  }   

  // stop
  bot.forward(0);
  
  // transform x by subtracting mean

//  for (i=0; i<2*_N_MAGCAL_DATA; i++) {
//    x_bar = x_bar + x[i][0];
//    y_bar = y_bar + x[i][1];
//  }
//  x_bar = x_bar/(2*_N_MAGCAL_DATA);
//  y_bar = y_bar/(2*_N_MAGCAL_DATA);
  for (i=0; i<2*_N_MAGCAL_DATA; i++) {
    x[i][0] = x[i][0] - x_bar;
    x[i][1] = x[i][1] - y_bar;
  }  
  
  // coeffs 
  float Suu = 0;
  float Suv = 0;
  float Svv = 0;
  float Suuu = 0;
  float Svvv = 0;
  float Suvv = 0;
  float Svuu = 0;
  for (i=0; i<2*_N_MAGCAL_DATA; i++) {
    Suu += x[i][0]*x[i][0];
    Suv += x[i][0]*x[i][1];
    Svv += x[i][1]*x[i][1];
    Suuu += x[i][0]*x[i][0]*x[i][0];
    Svvv += x[i][1]*x[i][1]*x[i][1];
    Suvv += x[i][0]*x[i][1]*x[i][1];
    Svuu += x[i][1]*x[i][0]*x[i][0];
  }
  
  // equation A*x = b
  Matrix A(2,2);
  Matrix b(2,1);
  A.assign(0,0,Suu);
  A.assign(0,1,Suv);
  A.assign(1,0,Suv);
  A.assign(1,1,Svv);
  b.assign(0,0, (Suuu+Suvv)/2.);
  b.assign(1,0, (Svvv+Svuu)/2.);
  
  // solve
  Matrix invA(2,2);
  Matrix u(2,1);
  A.setnIts(24);
  invA = A/(!A*0.1);
  u = invA*b;
  
  // centre
  bot.compass_x0 = u.get(0,0) + x_bar;
  bot.compass_y0 = u.get(1,0) + y_bar;
  
  // radius
  bot.compass_norm = sqrt(u.get(0,0)*u.get(0,0) 
                        + u.get(1,0)*u.get(1,0)
                        + (Suu+Svv)/(2.*_N_MAGCAL_DATA));  
                     
  
  // store calibration parameters in non-volatile memory
  bot.put_eeprom(bot.compass_x0_EEPROM_address, bot.compass_x0);
  bot.put_eeprom(bot.compass_y0_EEPROM_address, bot.compass_y0);
  bot.put_eeprom(bot.compass_norm_EEPROM_address, bot.compass_norm);

}






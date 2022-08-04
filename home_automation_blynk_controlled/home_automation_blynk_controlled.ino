/*************************************************************
Title         :   Home automation using blynk
Description   :   To control light's brigntness with brightness,monitor temperature , monitor water level in the tank through blynk app
Pheripherals  :   Arduino UNO , Temperature system, LED, LDR module, Serial Tank, Blynk cloud, Blynk App.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPLGlViPYUV"
#define BLYNK_DEVICE_NAME "Home Automation"
#define BLYNK_AUTH_TOKEN "asxKx3INzEdzbpjIgaT_dHj9xKzXSePI"


// Comment this out to disable prints 
//#define BLYNK_PRINT Serial

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "main.h"
#include "temperature_system.h"
#include "ldr.h"
#include "serial_tank.h"

char auth[] = BLYNK_AUTH_TOKEN;
bool heater_sw,inlet_sw,outlet_sw;
unsigned int tank_volume;

BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// This function is called every time the Virtual Pin 0 state changes
/*To turn ON and OFF cooler based virtual PIN value*/
BLYNK_WRITE(COOLER_V_PIN)
{
  int value = param.asInt();
  if(value)
  {
      cooler_control(ON);
      lcd.setCursor(7,0);
      lcd.print("Co_LR_ON"); 
  }
  else 
  {
    cooler_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("Co_LR_OFF"); 
  }
}
/*To turn ON and OFF heater based virtual PIN value*/
BLYNK_WRITE(HEATER_V_PIN )
{
  heater_sw = param.asInt();
  if(heater_sw)
  {
      heater_control(ON);
      lcd.setCursor(7,0);
      lcd.print("HT_R_ON"); 
  }
  else 
  {
    heater_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("HT_R_OFF"); 
  }
}
/*To turn ON and OFF inlet vale based virtual PIN value*/
BLYNK_WRITE(INLET_V_PIN)
{
  /* Reading the Value Present in virtual pin INLET_V_PIN and storing in it inlet_sw */
  inlet_sw = param.asInt();
  if(inlet_sw)
  {
      enable_inlet();
      lcd.setCursor(7,1);
      lcd.print("IN_FL_ON"); 
  }
  else 
  {
      disable_inlet();
      lcd.setCursor(7,0);
      lcd.print("IN_FL_OFF"); 
  }
}
/*To turn ON and OFF outlet value based virtual switch value*/
BLYNK_WRITE(OUTLET_V_PIN)
{
  outlet_sw = param.asInt();
  if(outlet_sw)
  {
      enable_outlet();
      lcd.setCursor(7,0);
      lcd.print("OT_FL_ON"); 
  }
  else 
  {
      disable_outlet();
      lcd.setCursor(7,0);
      lcd.print("OT_FL_OFF"); 
  }
}
/* To display temperature and water volume as gauge on the Blynk App*/  
void update_temperature_reading()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  /* Sending Temperature Reading to Temperature Guage for every 1 sec */
  Blynk.virtualWrite(TEMPERATURE_GAUGE, read_temperature());
  /* Sending Volume of water in the tank for every 1 sec */
  Blynk.virtualWrite(WATER_VOL_GAUGE , volume());
  
}

/*To turn off the heater if the temperature raises above 35 deg C*/
void handle_temp(void)
{
  if ((read_temperature() > float(35)) && heater_sw)
  {
      heater_sw = 0;
      heater_control(OFF);
      /* Send Nnotifications to Dashboard*/
      lcd.setCursor(7,0);
      lcd.print("HT_R OFF "); 
      /* Send Nnotifications to blynk Iot app*/
      Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN,"Temperature is above 35 celsius.\n");
      Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN,"Turning Off the Heater.\n");
      /* To turn off the heater widget button */
       Blynk.virtualWrite(HEATER_V_PIN,0);
      
        
  }
}

/*To control water volume above 2000ltrs*/
void handle_tank(void)
{
  if ( (tank_volume < 2000) && (inlet_sw == OFF))
  {
    enable_inlet();
    inlet_sw = ON;
    /* To Print Notifications on Dashboard */
    lcd.setCursor(7,1);
    lcd.print("IN_FL_ON"); 
    /* To Print Notifications on Blynk Iot App */
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN,"Water Level is less than 2000L. \n");
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN,"Water Flow Enabled.\n");
    /* To Reflect the status ON on the widget button of inlet valve */
    Blynk.virtualWrite(INLET_V_PIN, ON);
    
  }
  /* If volume of water is 3000 and if the inlet valve is ON disable Inflow */
  if ( (tank_volume == 3000) && (inlet_sw == ON) )
  {
    disable_inlet();
    inlet_sw = OFF;
    /* To Print Notifications on Dashboard */
    lcd.setCursor(7,1);
    lcd.print("IN_FL_OFF"); 
    /* To Print Notifications on Blynk Iot App */
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN,"Water Level is Full. \n");
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN,"Water Flow Disabled.\n");
    /* To Reflect the status OFF on the widget button of inlet valve */
    Blynk.virtualWrite(INLET_V_PIN, OFF);
    
  }
  

}


void setup(void)
{
    Blynk.begin(auth);
    lcd.init();                     
    lcd.backlight();
    lcd.clear();
    lcd.home();
    /* To Display Temperature*/
    lcd.setCursor(0,0);
    lcd.print("T=");
    /* To Display Volume*/
    lcd.setCursor(0,1);
    lcd.print("V=");
    /* Initialiasing Temperature System */
    init_temperature_system();
    init_ldr();
    /* Initialiasing Serial Tank */
    init_serial_tank();
    /* Update Temperature to Blynk App for every 1 sec */
    timer.setInterval(1000L, update_temperature_reading);
}

void loop(void) 
{
      /* To Run Blynk related function*/
      Blynk.run();
      /* To call setInterval at particular period*/
      timer.run();
      /* To Read Temperature and Display on Dashboard*/
      String temperature;
      temperature = String (read_temperature(), 2);
      lcd.setCursor(2,0);
      lcd.print(temperature);
       /* To Read Volume of the Tank and Display on Dashboard*/
      tank_volume = volume();
      lcd.setCursor(2,1);
      lcd.print(tank_volume);
       /* To Control the Garden Lightsbased on Light Intensity*/
      brightness_control();
       /* To Control Threshold Temperature of 35 degrees */
      handle_temp();
       /* To Control Volume of the water in the tank */
      handle_tank();
}

// include the library code:
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Bounce2.h>
#include <RunningAverage.h>
#include <EEPROM.h>


// define states
enum rele_states {
  off,
  t_on,
  h_on
};

enum lcd_states {
  set_h,
  set_t,
  disp
};

// defines pins
#define DHT22_PIN 10
#define BUTTON_T_P 8
#define BUTTON_T_M 6
#define BUTTON_H_P 5
#define BUTTON_H_M 4
#define LCD_RS 19
#define LCD_EN 18
#define LCD_D4 17
#define LCD_D5 16
#define LCD_D6 15
#define LCD_D7 14
#define RELE_T 11
#define RELE_H 12

//define constants
#define DELTA_T 3 //°C - Temperature less than...
#define DELTA_H 4 //%  - Humidity more than...
#define READ_INTERVAL 1000 //ms - Time between readings of the sensors
#define SET_INTERVAL  3000 //ms - Time to display the setup
#define ADDR_T 1 //EEPROM Address for temperature value
#define ADDR_H 2 //EEPROM Address for humidity value
#define RA_COUNT 10 //Running Average elements count

// Objects
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
DHT dht(DHT22_PIN, DHT22);
Bounce button_t_p = Bounce();
Bounce button_t_m = Bounce();
Bounce button_h_p = Bounce();
Bounce button_h_m = Bounce();
RunningAverage h(RA_COUNT);
RunningAverage t(RA_COUNT);

// Variables
rele_states rele_state = off;
lcd_states  lcd_state  = disp;
int h_set;
int t_set;
unsigned long read_metro = 0;
unsigned long disp_metro = 0;


void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(8, 2);
  
  //Set button
  pinMode(BUTTON_T_P,INPUT);
  pinMode(BUTTON_T_M,INPUT);
  pinMode(BUTTON_H_P,INPUT);
  pinMode(BUTTON_H_M,INPUT);
  //Set internal PULL-UP
  digitalWrite(BUTTON_T_P,HIGH);
  digitalWrite(BUTTON_T_M,HIGH);
  digitalWrite(BUTTON_H_P,HIGH);
  digitalWrite(BUTTON_H_M,HIGH);
  //Attach Debouncer
  button_t_p.attach(BUTTON_T_P);
  button_t_m.attach(BUTTON_T_M);
  button_h_p.attach(BUTTON_H_P);
  button_h_m.attach(BUTTON_H_M);
  
  //Set relay
  pinMode(RELE_T,OUTPUT);
  pinMode(RELE_H,OUTPUT);
  digitalWrite(RELE_T,LOW);
  digitalWrite(RELE_H,LOW);
  
  //Load eeprom
  t_set = EEPROM.read(ADDR_T)-127;
  h_set = EEPROM.read(ADDR_H)-127;
  
  //Benvenuto
  lcd.setCursor(0,0);
  lcd.print("* Ciao *");
  lcd.setCursor(0,1);
  lcd.print("Avvio...");
  
  delay(500);

  h.fillValue(dht.readHumidity(),RA_COUNT);
  t.fillValue(dht.readTemperature(),RA_COUNT);
  read_metro = millis()+READ_INTERVAL;
  
  delay(1500);
}

void loop() {
  // Sensor Read
  if (read_metro<millis()) {
    h.addValue(dht.readHumidity());
    t.addValue(dht.readTemperature());
    read_metro = millis()+READ_INTERVAL;
  }
  
  // Rele States
  switch (rele_state) {
    case off:
      if (t.getAverage()<=t_set-DELTA_T) {
        rele_state = t_on;
        digitalWrite(RELE_T,HIGH);
      } else if (h.getAverage()>=h_set+DELTA_H) {
        rele_state = h_on;
        digitalWrite(RELE_H,HIGH); 
      }
      break;
    case t_on:
      if (t.getAverage()>t_set) {
        rele_state = off;
        digitalWrite(RELE_T,LOW);
      }
      break;
    case h_on:
      if (t.getAverage()<=t_set-DELTA_T) {
        rele_state = t_on;
        digitalWrite(RELE_H,LOW);
        digitalWrite(RELE_T,HIGH);
      } else if (h.getAverage()<h_set) {
        rele_state = off;
        digitalWrite(RELE_H,LOW);
      }
      break;
  }


  //Check button state
  if (button_t_p.update() && button_t_p.read()) {
    t_set++;
    EEPROM.write(ADDR_T,t_set+127);
    disp_metro = millis()+SET_INTERVAL;
    lcd_state = set_t;
  }
  if (button_t_m.update() && button_t_m.read()) {
    t_set--;
    EEPROM.write(ADDR_T,t_set+127);
    disp_metro = millis()+SET_INTERVAL;
    lcd_state = set_t;
  }
  if (button_h_p.update() && button_h_p.read()) {
    h_set++;
    EEPROM.write(ADDR_H,h_set+127);
    disp_metro = millis()+SET_INTERVAL;
    lcd_state = set_h;
  }
  if (button_h_m.update() && button_h_m.read()) {
    h_set--;
    EEPROM.write(ADDR_H,h_set+127);
    disp_metro = millis()+SET_INTERVAL;
    lcd_state = set_h;
  }

  //Display states
  switch(lcd_state) {
    case disp:
      lcd_temperature();
      lcd_humidity();
      break;
    case set_t:
      if (disp_metro < millis()) lcd_state = disp;
      
      lcd_temperature();
      lcd.setCursor(0,1);
      lcd.print("<t:");
      lcd.print(t_set);
      lcd.print(" C  ");
      break;
    case set_h:
      if (disp_metro < millis()) lcd_state = disp;
      
      lcd.setCursor(0,0);
      lcd.print(">h:");
      lcd.print(h_set);
      lcd.print(" %  ");
      lcd_humidity();
      break;
  }  
}

void lcd_temperature() {
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(int(t.getAverage()));
  lcd.print(",");
  lcd.print(int(t.getAverage()*10)-int(t.getAverage())*10);
  lcd.print(" C");
}

void lcd_humidity() {
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.print(int(h.getAverage()));
  lcd.print(",");
  lcd.print(int(h.getAverage()*10)-int(h.getAverage())*10);
  lcd.print(" %");
}


                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

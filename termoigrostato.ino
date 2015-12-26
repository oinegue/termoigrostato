// include the library code:
#include <LiquidCrystal.h>
#include <dht.h>
#include <Bounce2.h>
#include <Metro.h>
#include <RunningAverage.h>
#include <EEPROM.h>

//LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(19, 18, 17, 16, 15, 14);

#define READ_TIME 1000
#define DHT22_PIN 10
dht DHT;
unsigned long read_metro = 0;

#define BUTTON_T_P 8
#define BUTTON_T_M 6
#define BUTTON_H_P 5
#define BUTTON_H_M 4
Bounce button_t_p = Bounce();
Bounce button_t_m = Bounce();
Bounce button_h_p = Bounce();
Bounce button_h_m = Bounce();

#define METRO_TIME 3000
#define METRO_NULL 0
#define METRO_H    1
#define METRO_T    2
unsigned long metro = 0;
int metro_type = METRO_NULL;

#define RELE_T 11
#define RELE_H 1

//Variabili
int h_set = 60;
int t_set = 60;
#define RA_COUNT 10
RunningAverage h(RA_COUNT);
RunningAverage t(RA_COUNT);
#define ADDRESS_T 1
#define ADDRESS_H 2

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
  t_set = EEPROM.read(ADDRESS_T)-127;
  h_set = EEPROM.read(ADDRESS_H)-127;
  
  //Benvenuto
  lcd.setCursor(0,0);
  lcd.print("* Ciao *");
  lcd.setCursor(0,1);
  lcd.print("Avvio...");
  
  delay(500);
  
  int chk = DHT.read22(DHT22_PIN);
  h.fillValue(DHT.humidity, RA_COUNT);
  t.fillValue(DHT.temperature, RA_COUNT);
  read_metro = millis()+READ_TIME;
  
  delay(1500);
}

void loop() {
  //Check button state
  if (button_t_p.update() && button_t_p.read()) {
    t_set++;
    EEPROM.write(ADDRESS_T,t_set+127);
    metro = millis()+METRO_TIME; metro_type = METRO_T;
  }
  if (button_t_m.update() && button_t_m.read()) {
    t_set--;
    EEPROM.write(ADDRESS_T,t_set+127);
    metro = millis()+METRO_TIME; metro_type = METRO_T;
  }
  if (button_h_p.update() && button_h_p.read()) {
    h_set++;
    EEPROM.write(ADDRESS_H,h_set+127);
    metro = millis()+METRO_TIME; metro_type = METRO_H;
  }
  if (button_h_m.update() && button_h_m.read()) {
    h_set--;
    EEPROM.write(ADDRESS_H,h_set+127);
    metro = millis()+METRO_TIME; metro_type = METRO_H;
  }
  
  if (metro>millis()) {
    switch(metro_type) {
      case METRO_T:
        lcd_temperature();
        lcd.setCursor(0,1);
        lcd.print("<t:");
        lcd.print(t_set);
        lcd.print(" C  ");
        break;
      case METRO_H:
        lcd.setCursor(0,0);
        lcd.print(">h:");
        lcd.print(h_set);
        lcd.print(" %  ");
        lcd_humidity();
        break;
    }
  } else {
    lcd_temperature();
    lcd_humidity();
  }
   
  if (read_metro<millis()) {
    int chk = DHT.read22(DHT22_PIN);
    h.addValue(DHT.humidity);
    t.addValue(DHT.temperature);
    read_metro = millis()+READ_TIME;
  }
  
  digitalWrite(RELE_T,t.getAverage() <  t_set                          ); //Accendi stufa se temperatura ambiente è inferiore a temperatura impostata
  digitalWrite(RELE_H,t.getAverage() >= t_set && h.getAverage() > h_set); //Accendi deumidificatore se stufa spenta e umidità ambiente è maggiore di umidità impostata
    
  
}

void lcd_temperature() {
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(int(t.getAverage()));
  lcd.print(",");
  lcd.print(int(t.getAverage()*10)-int(t.getAverage())*10);
  lcd.print(" C ");
}

void lcd_humidity() {
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.print(int(h.getAverage()));
  lcd.print(",");
  lcd.print(int(h.getAverage()*10)-int(h.getAverage())*10);
  lcd.print(" %");
}




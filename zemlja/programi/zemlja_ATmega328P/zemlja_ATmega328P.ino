/*
      -Fajl: zemlja_ATmega328P
      -Ucitava u ATmega328P mikrokontroler na UNO R3 WIFI ploci
      -Ocitava podatke sa senzora za zemljiste 7u1
      -Ispisuje vrednosti na LCD displeju 2x16 I2C
      -Salje podatke do ESP8266 na istoj ploci koji ih onda salje
       dalje preko WIFI do WEB aplikacije

       v.2
*/

#include <SoftwareSerial.h>     //The SoftwareSerial library allows serial communication on 
                                //other digital pins of an Arduino board
#include <Wire.h>               //This library allows you to communicate with I2C devices
#include <LiquidCrystal_I2C.h>  //The library allows to control I2C displays with functions 
                                //extremely similar to LiquidCrystal library

// Kreira lcd objekat za pristup LCD displeju
// sa parametrima: I2C adresa, broj karaktera u redu, broj redova
LiquidCrystal_I2C lcd(0x27, 16, 2);

//#define DEBUG
//#define DEBUG_1

#ifdef DEBUG
  int brojac_dugmeta = 0;
#endif


#define btnInputPin 2   // dugme za prolazak kroz razlicite prikaze LCD-a
#define ledPin 11       // test led za button input

//SoftwareSerial mySerial(2, 3);  // RX, TX; definise pinove za serijsku vezu sa 7u1 senzorom
SoftwareSerial mySerial(4, 5);  // RX, TX; definise pinove za serijsku vezu sa 7u1 senzorom

byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};  //komanda koja se salje senzoru
byte receivedData[19];    //niz u koji ce biti vraceni ocitani podaci sa senzora

unsigned int soilHumidity = 0;
unsigned int soilTemperature = 0;
unsigned int soilConductivity = 0;
unsigned int soilPH = 0;              // cuvaju int vrednosti ocitane sa senzora
unsigned int nitrogen = 0;            // posle preracunavanja iz dvobajtnog oblika
unsigned int phosphorus = 0;
unsigned int potassium = 0;

String strTemp;
String strVlaga;
String strPh;
String strEc;
String strN;
String strP;
String strK;

bool nova_temp = true;
bool nova_vlaga = true;
bool nov_ph = true;
bool nov_ec = true;
bool nov_n = true;
bool nov_p = true;
bool nov_k = true;

int trenutni_ekran = 1;      // broj ekrana koji se trenutno prikazuju
int period_slanja_upita = 1000;       // broj milisekundi izmedju dva slanja upita ka senzoru
int period_upit_ocitavanje = 1000;    // vreme od poslednjeg upita do ocitavanja prispelih podataka
unsigned long posl_upit = 0;          // vreme poslednjeg slanja upita ka senzoru dobijeno od millis()

volatile int buttonPressed = false;
unsigned long lastDebounceTime = 0; 
const unsigned long debounceDelay = 1000;

//bool ek_prom_ok = true;               // 
bool ceo_ekran = true;                // postavljeno na true kad treba da se iscrta ceo ekran
                                      // posle iscrtavanja celog ekrana setuje se na false
                                      
//String podaci = "'33.7', '33.7', '333', '3.7', '333', '333', '333', CURRENT_TIMESTAMP, NULL";
String podaci;

void setup() {
  pinMode(btnInputPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  mySerial.begin(4800);
  
  attachInterrupt(digitalPinToInterrupt(btnInputPin), buttonISR, FALLING);
  
  // inicijalizuje LCD ekran
  lcd.init();
  // ukljucuje pozadinsko osvetljenje LCD displeja
  lcd.backlight();
}

void loop() {

  // proverava dali je vreme za novi upit prema senzoru i ako jeste salje upit
  if((millis() - posl_upit) > period_slanja_upita){
    upit_senzoru();
    delay(1000);
    preuzmi_podatke();
    posl_upit = millis();
  }

  //ispisiOdgovor();  //ispisuje vracene vrednosti bajtova u odgovoru
  //ispisNaSerialMonitor();  //ispisuje izracunate vrednosti na Serial Monitoru

  // obradjuje prekide dugmeta
  if (buttonPressed) {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > debounceDelay) {
          ceo_ekran=true;
          promeni_ekran();
      //Serial.println("Button Pressed!");
      lastDebounceTime = currentTime;
    }
    buttonPressed = false;
  }
  
    switch (trenutni_ekran) {
    case 1:
          prikazi_ekran_1();
      break;
    case 2:
          prikazi_ekran_2();
      break;
    case 3:
          prikazi_ekran_3();
      break;
    }
  formiraj_string_podataka();
  //delay(200);
  posalji_na_esp();
    
}

// Ispisuje vrednosti svih 19 bajtova vracenih sa senzora u HEX formatu
void ispisiOdgovor() {
#ifdef DEBUG_1
  for (int i = 0; i < sizeof(receivedData); i++) {
    Serial.print(i);
    Serial.print("\t");
  }
  Serial.println("");
  for (int i = 0; i < sizeof(receivedData); i++) {
    Serial.print(receivedData[i], HEX);
    Serial.print("\t");
  }
  Serial.println("");
  Serial.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
#endif
}

// Ispisuje vrednosti sa senzora na IDE serial monintor
void ispisNaSerialMonitor() {
  Serial.print("Vlaznost zemljista: ");
  Serial.println((float)soilHumidity / 10.0);
#ifdef DEBUG
  Serial.println("  >> DEBUG");
  Serial.print("  >> receivedData[3]: ");
  Serial.println(receivedData[3], HEX);
  Serial.println("  >> END DEBUG");
#endif
  Serial.print("Temperatura zemljista: ");
  Serial.println((float)soilTemperature / 10.0);
#ifdef DEBUG
  Serial.println("  >> DEBUG");
  Serial.print("  >> receivedData[3]: ");
  Serial.println(receivedData[3], HEX);
  Serial.println("  >> END DEBUG");
#endif
  Serial.print("Provodljivost zemljista: ");
  Serial.println(soilConductivity);
  Serial.print("pH zemljista: ");
  Serial.println((float)soilPH / 10.0);
  Serial.print("Azot: ");
  Serial.println(nitrogen);
  Serial.print("Fosfor: ");
  Serial.println(phosphorus);
  Serial.print("Kalijum: ");
  Serial.println(potassium);
  Serial.println("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
  return;
}

// EKRAN_1
// Ispisuje vrednosti Temp i Vlagu na LCD ekran
void prikazi_ekran_1() {
  
  if(ceo_ekran){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Temp   | Vlaga");
  lcd.setCursor(6, 1);
  lcd.print((char) 223);
  lcd.setCursor(7, 1);
  lcd.print("C");
  lcd.setCursor(8, 1);
  lcd.print("|");
  lcd.setCursor(15, 1);
  lcd.print("%");   
  ceo_ekran = false;
  } 
  
    if ((soilTemperature / 10.0) >= 0 ) {
      lcd.setCursor(0, 1);
      lcd.print("     ");
      lcd.setCursor(1, 1);
      lcd.print((soilTemperature / 10.0), 1);
    } else {
      //negativne temperature
    }    

  lcd.setCursor(9, 1);
  lcd.print("      ");
  lcd.setCursor(10, 1);
  lcd.print((soilHumidity / 10.0), 1);
 
  delay(2000);
}

// EKRAN_2
// Ispisuje vrednosti PH i EC na LCD ekran
void prikazi_ekran_2() {
  if(ceo_ekran){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" PH  | EC");
  lcd.setCursor(11, 0);
  lcd.print("mS/cm");
  lcd.setCursor(4, 1);
  lcd.print(" |");
  ceo_ekran = false;
  }

    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.setCursor(1, 1);
    lcd.print((soilPH / 10.0),1);

    lcd.setCursor(7, 1);
    lcd.print("        ");
    lcd.setCursor(7, 1);
    lcd.print(soilConductivity);
  
    delay(2000);
}

// EKRAN_3
// Ispisuje vrednosti Н, P i K na LCD ekran
void prikazi_ekran_3() {
  if(ceo_ekran){
  lcd.clear();
  lcd.setCursor(0, 0);
  // prvi red
  lcd.print("  N-P-K  [mg/Kg]");
  ceo_ekran = false;
  }

    lcd.setCursor(0, 1);
    lcd.print("      ");
    lcd.setCursor(0, 1);
    lcd.print(nitrogen);

    lcd.setCursor(6, 1);
    lcd.print("      ");
    lcd.setCursor(6, 1);
    lcd.print(phosphorus);

    lcd.setCursor(12, 1);
    lcd.print("    ");
    lcd.setCursor(12, 1);
    lcd.print(potassium);
    
    delay(2000);
}

// Salje komandu senzoru za ocitavanje novih podataka
void upit_senzoru() {
  // prazni bafer
  while (mySerial.available() > 0) {
    mySerial.read();
  }
  
  mySerial.write(queryData, sizeof(queryData));  // Salje komandu (upit) ka 7u1 senzoru
  mySerial.flush();
  return;
}

// Ocitava vracene podatke sa senzora i proverava dali ima promene u odnosu na
// prethodno ocitane podatke
void preuzmi_podatke() {
  
  if (mySerial.available() >= sizeof(receivedData)) {        // Proverava dali se dovoljno bajtova vratilo u odgovoru za ocitavanje
    mySerial.readBytes(receivedData, sizeof(receivedData));  // Ocitava vracene bajtove u receivedData niz

    // Izracunava vrednosti pojedinih parametara vracenih sa senzora i stavlja u privremenu "n_" promenljivu
    // Vrednost svakog parametra vraca se u dva bajta iz kojih se onda izracunava vrednost po
    // formuli [ MSB << 8 | LSB ]
    unsigned int  n_soilHumidity = (receivedData[3] << 8) | receivedData[4];
    unsigned int  n_soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int  n_soilConductivity = (receivedData[7] << 8) | receivedData[8];
    unsigned int  n_soilPH = (receivedData[9] << 8) | receivedData[10];
    unsigned int  n_nitrogen = (receivedData[11] << 8) | receivedData[12];
    unsigned int  n_phosphorus = (receivedData[13] << 8) | receivedData[14];
    unsigned int  n_potassium = (receivedData[15] << 8) | receivedData[16];

    if(n_soilHumidity != soilHumidity){
      soilHumidity = n_soilHumidity;
      strVlaga = String((soilHumidity / 10.0), 1);
      nova_vlaga = true;
    }
    if(n_soilTemperature != soilTemperature){
      soilTemperature = n_soilTemperature;
      strTemp = String((soilTemperature / 10.0), 1);
      nova_temp = true;
    }
    if(n_soilPH != soilPH){
      soilPH = n_soilPH;
      strPh = String((soilPH / 10.0),1);
      nov_ph = true;
    }
    if(n_soilConductivity != soilConductivity){
      soilConductivity = n_soilConductivity;
      strEc = String(soilConductivity);
      nov_ec = true;
    }
    if(n_nitrogen != nitrogen){
      nitrogen = n_nitrogen;
      strN = String(nitrogen);
      nov_n = true;
    }
    if(n_phosphorus != phosphorus){
      phosphorus = n_phosphorus;
      strP = String(phosphorus);
      nov_p = true;
    }
    if(n_potassium != potassium){
      potassium = n_potassium;
      strK = String(potassium);
      nov_k = true;
    }
  }
  return;
}

// Setuje vreme kada je pin kontrolnog dugmeta otisao na LOW
// Poziva se iz prekida
void buttonISR() {
  buttonPressed = true; // Set a flag for the button press
}

// Izvrsava se u glavnoj petlji
// Ceka uslove da se promeni ekran da displeju
void promeni_ekran(){

        if (trenutni_ekran < 3) {
          ++trenutni_ekran;
         }else{
          trenutni_ekran = 1;
         }
}

void posalji_na_esp(){
  Serial.println(podaci);
}

void formiraj_string_podataka(){
  //podaci = "'33.7', '33.7', '333', '3.7', '333', '333', '333', CURRENT_TIMESTAMP, NULL";
  podaci ="";
  
  String navodnik = "'";
  String tri_navodnika = "', '";
  String navodnik_zarez ="', ";
  String kraj = "CURRENT_TIMESTAMP, NULL";
  
  podaci += navodnik;
  podaci += strVlaga;
  podaci += tri_navodnika;
  podaci += strTemp;
  podaci += tri_navodnika;
  podaci += strEc;
  podaci += tri_navodnika;
  podaci += strPh;
  podaci += tri_navodnika;
  podaci += strN;
  podaci += tri_navodnika;
  podaci += strP;
  podaci += tri_navodnika;
  podaci += strK;
  podaci += navodnik_zarez;
  podaci += kraj;
}

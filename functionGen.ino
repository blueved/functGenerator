// VERSION 1.1
// 11/19/2026

#include <LiquidCrystal_I2C.h> // Library for LCD
#include <AD9833.h>
#include <Pushbutton.h>
#include <Wire.h>
#include <SPI.h>

#define WAVE_BTN_PIN 2
#define FINC_BTN_PIN 3
#define FDEC_BTN_PIN 4
#define FREQ_MULT_PIN 5
#define MCP4131_PIN 9

Pushbutton WaveButton(WAVE_BTN_PIN);
Pushbutton FIncButton(FINC_BTN_PIN);
Pushbutton FDecButton(FDEC_BTN_PIN);
Pushbutton FMultiButton(FREQ_MULT_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
AD9833 AD(10);  //HW SPI (FNC:pin10, DAT:pin11, CLK:pin13)

int WaveTable[3] = {AD9833_SINE, AD9833_SQUARE1, AD9833_TRIANGLE};
unsigned long FrequencyIncrementTable[3] = {1, 10, 100};    // frequency increment
String WaveStringTable [3] = {"SINE", "SQUR", "TRIG"};

int i = 0;                    // Index For The WaveTable Array
int incrementIndex = 0;       // Index for the FrequencyIncrementTable
unsigned long Freq = 1000;    // Initial f: 1KHz
const int minFreq = 1;        // Minimum freq value;
const int FreqChange = 1000;  // The push button will change f by this amount * the increment from the frequencyIncrementTable
const unsigned long  MaxFreq = 20000000; // Max 20MHZ
int rawValue;                 // Current reading of frequency POTAR value
int oldValue;                 // Previous reading of frequency POTAR value
float multiFactor = 1;        // Frequency potar  factor
const int DELTA = 3;          // Threshold of the freqency potar read
int numberOfLoop = 0; 
const String emptyLine = "                       ";
byte address = 0x00;
unsigned int wiperValue;
String formatedFreq = "100";

void setup() {
  Serial.begin(9600);

  // Initialize the display
  // if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
  //   Serial.println("SSD1306 allocation failed");
  //   for (;;); // Loop forever if the display initialization fails
  // }
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight 
  int column_index = 0;
  int row_index = 0;
  lcd.setCursor(column_index, row_index);
  lcd.print("Hello World!");

  pinMode (MCP4131_PIN, OUTPUT);
  digitalWrite(MCP4131_PIN, HIGH);
  SPI.begin();
  AD.begin();
  
  rawValue = analogRead(A0);
  rawValue = analogRead(A0); 
  oldValue = rawValue; 
  Freq += rawValue;
  AD.setFrequency(Freq, 0); 
  AD.setWave(WaveTable[i]);
  //updateDisplayFrequency();
  //updateDisplayWaveForm();  
  updateDisplay();

  numberOfLoop = 0;

  //Serial.println(" "); Serial.print("Wave Index table is: "); Serial.println(i);
  //Serial.print("AD9833_LIB_VERSION: ");Serial.println(AD9833_LIB_VERSION);
  //Serial.print("Analog Read: "); Serial.print(rawValue); Serial.print(" f: "); Serial.print(Freq); Serial.println(" Hz");
  //Serial.print("Freq Mult: "); Serial.println(digitalRead(FREQ_MULT_PIN));
  
  wiperValue = 1;
  //Serial.print("Setting wave table "); 
  //Serial.print(i); Serial.print(" --> " );Serial.print(WaveStringTable[i]); Serial.print("  wiper val: ");Serial.println(wiperValue);
  // Serial.print("DEBUG: #9: "); Serial.print(digitalRead(MCP4131_PIN));Serial.print("  #10: ");Serial.println(digitalRead(10));

  // MCP431Potar.writeWiper(wiperValue);
  digitalPotWrite(wiperValue);

  Serial.println("buttons reads:");
  Serial.print("- Freq Increase: ");  Serial.println(digitalRead(FINC_BTN_PIN)==LOW); // We are active on HIGH
  Serial.print("- Freq Decrease: ");  Serial.println(digitalRead(FDEC_BTN_PIN)==LOW);
  Serial.print("- Freq multiplier: ");Serial.println(digitalRead(FREQ_MULT_PIN)==LOW);
}
 
void loop(){
  if(false){
     return;
  }
  if (WaveButton.getSingleDebouncedPress()){
    i += 1 ;
    if(i == 3){
      i = 0;      
    } 
    wiperValue = 1;   // Adjust the gain based on the signal (the square wave jumps to 10 times the other signals)
    if(i == 1){
        wiperValue = 125;        
    }
    
    //Serial.print("*** Setting wave table "); 
    //Serial.print(i); Serial.print(" --> " );Serial.print(WaveStringTable[i]); Serial.print("  wiper val: ");Serial.println(wiperValue);
    updateDisplay();    
    //Serial.print("\n-- DEBUG: #9: "); Serial.print(digitalRead(9));Serial.print("  #10: ");Serial.println(digitalRead(10));
    digitalPotWrite(wiperValue);
    //Serial.print("-- DEBUG: #9: "); Serial.print(digitalRead(9));Serial.print("  #10: ");Serial.println(digitalRead(10));
    AD.setWave(WaveTable[i]);    
  }
  if(FMultiButton.getSingleDebouncedPress()){
    incrementIndex += 1 ;
    if(incrementIndex > 2){
      incrementIndex = 0;
    }
    Serial.print("Increment value pressed index:"); Serial.print(incrementIndex); Serial.print(" val: "); Serial.println(FrequencyIncrementTable[incrementIndex]);
    updateDisplay();
  }

  numberOfLoop = 0;
  while(digitalRead(FINC_BTN_PIN) == HIGH){   
    Serial.print("*** INC by "); 
    Serial.print(FrequencyIncrementTable[incrementIndex]);
    Serial.print(" delta: "); Serial.print(FreqChange); 
    
    unsigned long df = FreqChange * FrequencyIncrementTable[incrementIndex];
    Freq += df;
    Serial.print(" ==> f change ");Serial.print( df ); 
    Serial.print(" ==>new f: "); Serial.print(Freq); 
    Serial.println(""); 
    if(Freq >= MaxFreq){
      Freq = MaxFreq;
    }
    AD.setFrequency(Freq, 0);
    updateDisplay();
    Serial.print("*** INC  Frequence "); Serial.print(" f: "); Serial.print(Freq); Serial.println(" Hz");    
    numberOfLoop += 1;
    if(numberOfLoop >= 3){
      delay(100);
    }else{
      delay(800);
    }
  }
  
  while (digitalRead(FDEC_BTN_PIN) == HIGH){
    Serial.println("*** DECREMENTING  Frequence "); 
    Freq -= FreqChange * FrequencyIncrementTable[incrementIndex];    
    if(Freq < 0){
      Freq = minFreq;
    }
    AD.setFrequency(Freq, 0);
    Serial.print("*** DEC  Frequence "); Serial.print(" f: "); Serial.print(Freq); Serial.println(" Hz");
    updateDisplay();
        
    numberOfLoop += 1;
    if(numberOfLoop >= 3 ){
      delay(100);
    }else{
      delay(800);
    }
  }  
  numberOfLoop = 0;
  readPotentiometer ();
  
}

void readPotentiometer(){
  rawValue = analogRead(A0) * multiFactor; 
  // rawValue = analogRead(A0) * multiFactor; 
  int delta = rawValue - oldValue;
  if(delta >= DELTA ){
    Freq += delta;
    Serial.print("POT has INCREASED "); Serial.print(oldValue); Serial.print( " TO ");Serial.print(rawValue);Serial.print(" ("); Serial.print(delta);  Serial.print(") f: "); Serial.print(Freq); Serial.println(" Hz");
    oldValue = rawValue;     
    AD.setFrequency(Freq, 0);
    updateDisplay();

  }else if ( delta <= -DELTA){
    Freq += delta;
    if(Freq <= 0 ){
      Freq = minFreq;       
    }
    Serial.print("POT has DECREASED "); Serial.print(oldValue); Serial.print( " TO ");Serial.print(rawValue); Serial.print(" f: "); Serial.print(Freq); Serial.println(" Hz");
    oldValue = rawValue; 
    AD.setFrequency(Freq, 0);
    updateDisplay();      
  }
}

// void updateDisplayFrequency(long frequence, int waveFormIndex){
void updateDisplayFrequency(){
  String frequencyString = String(Freq) + "HZ";
  // Format the number:  
  if (Freq >= 1000 ){
    frequencyString =  String(float(Freq / 1000)) +" KHZ";
  } 
  lcd.setCursor(0, 1); 
  lcd.print(emptyLine);  
  lcd.setCursor(0, 1);   
  lcd.print(frequencyString);
}

// Formats a frequency into buf, e.g.:
//   532      -> "532 Hz"
//   1500     -> "1,500 KHz"
//   12345    -> "12,345 KHz"
//   1234567  -> "1,234567 MHz"
String formatFrequency(unsigned long freq) {
  char buf[20];

  if (freq < 1000UL) {
    snprintf(buf, sizeof(buf), "%lu Hz", freq);
  } else if (freq < 1000000UL) {
    snprintf(buf, sizeof(buf), "%lu,%03lu KHz", freq / 1000UL, freq % 1000UL);
  } else {
    snprintf(buf, sizeof(buf), "%lu,%06lu MHz", freq / 1000000UL, freq % 1000000UL);
  }
  return String(buf);
}

void updateDisplayWaveForm(){  
  lcd.setCursor(0, 0); 
  lcd.print(emptyLine);   
  lcd.setCursor(0, 0);
  String test = WaveStringTable[i] +  " - Inc : " + FrequencyIncrementTable[incrementIndex];// + " - Inc : " + FrequencyIncrementTable[incrementIndex];
  lcd.print(test);
}

void updateDisplay() {
  lcd.clear();
  // First line
  lcd.setCursor(0, 0);
  lcd.print(currentWave(i)+  " - Inc : " + FrequencyIncrementTable[incrementIndex]);

  // Second line
  formatedFreq =formatFrequency(Freq);
  lcd.setCursor(0, 1);
  lcd.print(formatedFreq);

}

String currentWave(int i) {
  String val = "";
  switch (i) {
    case 1:
      val = "SQUR";
      break;
    case 2:
      val = "TRIG";
      break;
    default:
      val = "SINE";
  }
  return val;
}

void digitalPotWrite(unsigned int value){
  Serial.print("-- SETTING MCP4131 TO: "); Serial.println(value);
  digitalWrite(MCP4131_PIN, LOW);
  Serial.print("--->: #9: "); Serial.print(digitalRead(MCP4131_PIN));
  SPI.transfer(address);
  SPI.transfer(value);
  // MCP431Potar.writeWiper(value);
  digitalWrite(MCP4131_PIN, HIGH);
  Serial.print("   apres:"); Serial.println(digitalRead(MCP4131_PIN));
}

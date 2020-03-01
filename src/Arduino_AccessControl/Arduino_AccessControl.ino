/*
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 */


/*
   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino  Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3  Pro Mini   Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin      Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9       D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10      D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11      D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12      D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13      D13        ICSP-3           15
*/

#include <SPI.h>                // RC522 Module uses SPI protocol
#include <MFRC522.h>            // Library for Mifare RC522 Devices
#include <SoftwareSerial.h>
SoftwareSerial softSerial(2,3); // RX, TX
String inputString = "";        // A String to hold incoming data
bool stringComplete = false;    // whether the string is complete

const uint32_t baud = 9600;

#define ARM_LED_PIN 4
#define DISARM_LED_PIN 5

#define RESET_WIFI_PIN 8
#define ENABLE_WIFI_PIN 7

#define UNDEFINED 0
#define ARM 1
#define DISARM 2
int segMode = UNDEFINED; // Alarm status Armed/Disarmed.

uint8_t successRead;     // Variable integer to keep if we have Successful Read from Reader

byte readCard[4];        // Stores scanned ID read from RFID Module
String tag;              // Stores scanned ID read from RFID Module as String

// Create MFRC522 instance.
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {

  // Pin config;
  pinMode(ARM_LED_PIN, OUTPUT);
  pinMode(DISARM_LED_PIN, OUTPUT);
  pinMode(RESET_WIFI_PIN, OUTPUT);
  pinMode(ENABLE_WIFI_PIN, OUTPUT);

  // Initial led mode;
  digitalWrite(ENABLE_WIFI_PIN, LOW);
  digitalWrite(RESET_WIFI_PIN, LOW);
  digitalWrite(ARM_LED_PIN, LOW);  
  digitalWrite(DISARM_LED_PIN, LOW);


  //Protocol Configuration
  Serial.begin(baud);      // Initialize serial communications with PC
  softSerial.begin(baud);  // Inicializs serial cominications with ESP-01s
  SPI.begin();             // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();      // Initialize MFRC522 Hardware

  //If you set Antenna Gain to Max it will increase reading distance
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  Serial.println(F("Access Control Example v0.1"));   // For debugging purposes
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

  Serial.println(F("Waiting for Wifi module"));  
  digitalWrite(ENABLE_WIFI_PIN, HIGH);
  digitalWrite(RESET_WIFI_PIN, HIGH);
  while(!stringComplete) {
    leerComando();
    if (stringComplete) {
      if (inputString.indexOf("Wifi module started") >= 0)
        goto WifiConected;
      Serial.println(inputString);
      inputString = "";
      stringComplete = false;
    }
    Serial.print(".");
  }
WifiConected:
  Serial.println(inputString);
  inputString = "";
  stringComplete = false;
  // Tell wifi module get security status.
  softSerial.write("GetSecurityStatus\n");

  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
}


///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0    
    setStatusLeds();
    leerComando();
    if (stringComplete) {
      ejecutarComando(inputString);
      inputString = "";
      stringComplete = false;
    }
  } while (!successRead);   //the program will not go further while you are not getting a successful read
  sendTag();
}

void sendTag() {
  softSerial.write(String(tag + "\n").c_str());
}

void leerComando() {
  while (softSerial.available()) {
    // get the new byte:
    char inChar = (char)softSerial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
      stringComplete = true;
    else
      // add it to the inputString: I don't want a \n at end.
      inputString += inChar;
  }
}

void ejecutarComando(String comando) {
  if (comando == String("Disarm")) {
    segMode = DISARM;
  } else if (comando == String("Armed")) {
    segMode = ARM;
  } else
    Serial.write(String(comando + "\n").c_str());
}

void setStatusLeds() {
  if (segMode == DISARM) {
    digitalWrite(DISARM_LED_PIN, HIGH);
    digitalWrite(ARM_LED_PIN, LOW);
  } else if (segMode == ARM) {
    digitalWrite(DISARM_LED_PIN, LOW);
    digitalWrite(ARM_LED_PIN, HIGH);
  } else {
    digitalWrite(DISARM_LED_PIN, HIGH);
    digitalWrite(ARM_LED_PIN, HIGH);
  }
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading

  char str[32] = "";
  array_to_string(readCard, 4, str);
  tag = String(str);

  return 1;
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    while (true); // do not go further
  }
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

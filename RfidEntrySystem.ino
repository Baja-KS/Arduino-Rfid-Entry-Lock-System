/*
 * ----------------------------------------------------------------------------
 * This sketch uses the MFRC522 library ; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * This sketch show a simple locking mechanism using the RC522 RFID module.
 * ----------------------------------------------------------------------------
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <TimeLib.h>
#include <Ethernet.h>
#include <HttpClient.h>

#define RST_PIN         49           // Configurable, see typical pin layout above
#define SS_PIN          53          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

String read_rfid;                   // Add how many you need and don't forget to include the UID.
String ok_rfid_1="f47fef20";        // This is for my main RFID Card. aka. The one I will be using to turn on my PC. Can also be used to shut it down if you want to.
String ok_rfid_2="dab93247";        // This is for the RFID Keyfob. aka. Shutdown Keyfob. Not advisable tho. Just shutdown your PC normally.
int lock = 7;// For the Card.
int mod=0;//0-standby,1-ulaz,2-sluzbeno,3-privatno,4-pauza,5-izlaz
bool triggeredByRFID=false;
//bool modExpired=false;
bool modChanged=false;
//int lock2 = 7;                      // For the Keyfob.
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
int taster = analogRead(A0); // pin tastera
int ostaliTasteri=analogRead(A1);

int LEDdioda = 8; // pin LED dioda
// ova promenjiva se menja pritiskom na taster
int stanjetastera = 0; //promenjiva koj
int stanjeOstalihTastera=0;

//defunct
struct tm* timeinfo;
time_t rawtime;


EthernetClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 119);
int httpPort=80;
char hostName[]="192.168.1.112";



/*
 * Initialize.
 */
void setup() {
    Serial.begin(9600);         // Initialize serial communications with the PC
    while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();                // Init SPI bus
    mfrc522.PCD_Init();         // Init MFRC522 card

    //Choose which lock below:
    pinMode(lock, OUTPUT);
//    pinMode(lock2, OUTPUT);
    pinMode(LEDdioda, OUTPUT);
    pinMode(taster, INPUT);
    pinMode(ostaliTasteri,INPUT);
    lcd.init();                      // initialize the lcd 
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    Ethernet.begin(mac, ip);
//    if (Ethernet.begin(mac) == 0) {
//      Serial.println("Failed to obtaining an IP address using DHCP");
//      Ethernet.begin(mac, ip);
//  }

}
/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    read_rfid="";
    for (byte i = 0; i < bufferSize; i++) {
        read_rfid=read_rfid + String(buffer[i], HEX);
    }
}
//0-standby,1-ulaz,2-sluzbeno,3-privatno,4-pauza,5-izlaz
String modMapIntToString(int modInt)
{
  switch(modInt)
  {
    case 0:
      return "SPREMNO!";
    case 1:
      return "ULAZ!";
    case 2:
      return "SLUZBENO!";
    case 3:
      return "PRIVATNO!";
    case 4:
      return "PAUZA!";
    case 5:
      return "IZLAZ!";
    
  }
}

void open_lock(bool successful=true,int entryMod=0) {
  //Use this routine when working with Relays and Solenoids etc.
  lcd.clear();
  if(successful)
  {
    digitalWrite(lock,HIGH);
    digitalWrite(LEDdioda, HIGH);
    if(!entryMod)
    { 
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print("Vrata Otvorena");
      lcd.setCursor(4,1);
      lcd.print("IZADjITE!");
      delay(2000);
    }
    else
    {
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print(modMapIntToString(entryMod));
      lcd.setCursor(4,1);
      lcd.print("USPESNO!");
      delay(2000);
    }
    
    digitalWrite(LEDdioda, LOW);
    digitalWrite(lock,LOW);
  }
  else
  {
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Greska");
    lcd.setCursor(0,1);
    lcd.print("Neispravan ID");
    delay(2000);
  }
//  modChanged=true;
}
void displayEntry(int entryMod)
{
//  char* printTime;
//  sprintf(printTime,"asdf");
   
  if(modChanged)
    lcd.clear();
  if(mod)
  {
  lcd.backlight();
  lcd.setCursor(5,0);
  lcd.print(String(hour())+":"+String(minute()));
  lcd.setCursor(4,1);
  lcd.print(modMapIntToString(entryMod));
  }
  else
  {
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Vrata Zatvorena");
    lcd.setCursor(4,1);
    lcd.print("SPREMNO!");
  }
//  delay(2000);
}
//bool modExpire(void *argument)
//{
//  mod=0;
//  lcd.clear();
//  lcd.backlight();
//  lcd.setCursor(0,0);
//  lcd.print("Vrata Zatvorena");
//  lcd.setCursor(4,1);
//  lcd.print("SPREMNO!");
//  modExpired=true;
//  return false;
//}
//void modExpireRead()
//{
//  mod=0;
//  lcd.clear();
//  lcd.backlight();
//  lcd.setCursor(0,0);
//  lcd.print("Vrata Zatvorena");
//  lcd.setCursor(4,1);
//  lcd.print("SPREMNO!");
//  modExpired=true;
//}
void logEntryDB(bool validEntry=true)
{
  String httpMethod="GET";
  String pathName="/rfid/logEntry.php";
  String queryString="?mode="+(validEntry ? String(mod) : String("-1"))+"&rfid="+String(read_rfid);
  if(client.connect(hostName, httpPort)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println("GET " + pathName + queryString + " HTTP/1.1");
    client.println("Host: " + String(hostName));
    client.println("Connection: close");
    client.println(); // end HTTP header

    Serial.println("Response:");
    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
          Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }
}
void readRFID(int entryMod=0)
{
  modChanged=false;//ako ne procita nece da clearuje display
  if ( ! mfrc522.PICC_IsNewCardPresent())
        {
                triggeredByRFID=false;
                return;
        }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        {
                triggeredByRFID=false;
                return;
        }
  
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
//    Serial.println(read_rfid,entryMod);read_rfid==ok_rfid_1 || read_rfid==ok_rfid_2
    if (rfidValidate()) {
      //ok, open the door.
      triggeredByRFID=true;
      open_lock(true,entryMod);
      logEntryDB();
    }
    else
      {
        open_lock(false,entryMod);
        logEntryDB(false);
      }
    modChanged=true;//ako procita obelezava da display treba da se clear prvo pa onda ispisuje
    Serial.println(read_rfid);
}
bool readRFIDCallback(void *argument)
{
  readRFID(mod);
  return false;
}

bool rfidValidate()
{
  bool valid=false;
  String httpMethod="GET";
  String pathName="/rfid/validateEntry.php";
  String queryString="?rfid="+String(read_rfid);
  if(client.connect(hostName, httpPort)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println("GET " + pathName + queryString + " HTTP/1.1");
    client.println("Host: " + String(hostName));
    client.println("Connection: close");
    client.println(); // end HTTP header

    char previous='-';
    Serial.println("Response:");
    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        if(previous=='#')
          {
            Serial.print(c);
            c=='1'? valid=true : valid=false;
          }
        previous=c;
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }
  return valid;
}


void loop() {
  stanjetastera = analogRead(A0);
  stanjeOstalihTastera=analogRead(A1);
  time (&rawtime);
  timeinfo = localtime (&rawtime);

//  Serial.println(stanjeOstalihTastera);
  delay(100);

  if ( !triggeredByRFID and stanjetastera > 0   ) {
  mod=0;
  open_lock();
//  modChanged=true;
  }
  //0-standby,1-ulaz,2-sluzbeno,3-privatno,4-pauza,5-izlaz
  if (stanjeOstalihTastera >950  and (stanjeOstalihTastera <1100 ))
    {
      mod=1;
      modChanged=true;
    }
  if (stanjeOstalihTastera >750  and (stanjeOstalihTastera <850 ))
    {
      mod=2;
      modChanged=true;
    }
  if (stanjeOstalihTastera >550  and (stanjeOstalihTastera <750 ))
   {
      mod=3;
      modChanged=true;
    }
  if (stanjeOstalihTastera >350 and (stanjeOstalihTastera <450 ))
    {
      mod=4;
      modChanged=true;
    }
  if (stanjeOstalihTastera >150  and (stanjeOstalihTastera <250 ))
    {
      mod=5;
      modChanged=true;
    }


  displayEntry(mod);
  readRFID(mod);
//  modChanged=false;
}

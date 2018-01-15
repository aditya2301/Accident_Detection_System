 /*********************
 *03 to GPS Module TX*
 *08 to GPS Module RX*
 *********************/

#include<SoftwareSerial.h>
#include <stdlib.h>
#include <TinyGPS.h>
SoftwareSerial ser(6,5);
SoftwareSerial ss(8,3);//pin 8(rx) connect to gps TX
//SoftwareSerial MySerial(10, 11);//(pin 11(tx) to rx to gsm and 10 
TinyGPS gps;
String apiKey = "NEWO1WEMUYM1BOX1";
String id="1ds14ec011";
int s1=0,s2=0,s3=0,s4=0,s5=0,s6=0;//vcc at 2nd row 3rd pin and two gnd at 2nd row 1st  

byte val = 0;
int statePin = LOW;
int THRESHOLD =120;
void gpsdump(TinyGPS &gps);
void printFloat(double f, int digits = 2);
static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);
void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
 
 // MySerial.begin(9600);
  // set the data rate for the SoftwareSerial port
  ss.begin(9600);
  delay(1000);
  ser.begin(115200);
  ser.println("AT+RST");
  
}

void loop() // run over and over
{
   s1 =analogRead(A0);
  // print out the value you read:

 s2= analogRead(A1);

 s3= analogRead(A2);

 s4=analogRead(A3);

 s5=analogRead(A4);

 s6=analogRead(A5);
 gps_init();
if((s1>20) or (s2>20) or (s3>20) or (s4>20)  or (s5>20) or (s6>20))
{
  Serial.println(" accident");
   Serial.println("Front left impact");
  Serial.println(s1);
  Serial.println("Front right impact");
Serial.println(s2);
Serial.println("Right impact");
Serial.println(s3);
Serial.println("Left impact");
Serial.println(s4);
Serial.println("Back right impact");
Serial.println(s5);
Serial.println("Back left impact");
Serial.println(s6);
    gps_init();
   // SendMessage();
    esp_server();
}
  delay(500);        // delay in between reads for stability
}
  

void esp_server()
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
  delay(1000);
  if(ser.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }

  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(s1);
  getStr +="&field2=";
  getStr += String(s2);
  getStr +="&field3=";
  getStr += String(s3);
  getStr +="&field4=";
  getStr += String(s4);
  getStr +="&field5=";
  getStr += String(s6);
  getStr +="&field6=";
  getStr += String(s5);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);
  delay(500);
 // ser.println(getStr);
  if(ser.find(">"))
  {
      ser.print(getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }

  // this delay is required before disabling the ESP8266 chip
  delay(16000); 
}
void gps_init()
{
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  print_date(gps);
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  
  smartdelay(1000);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}


#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <RealTimeClockDS1307.h>
#include <LiquidCrystal.h>

float voltage, current, power;
float tegangan, total_voltage, arus, total_current;
unsigned char sec, minute, hour;
unsigned long time;
long int adc_tegangan, adc_arus;

// variabel untuk membaca waktu (real time) dari DS1307 
int hours = 0;
int minutes = 0;
int seconds = 0;
int dates = 0;
int months = 0;
int years = 0;

// variabel pengatur tombol
int adc_key_in    = 0;
int time_push_data = 1;
unsigned long time2 = 0;
int i_maks = 0;

// variabel Ethernet
byte mac [] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip (192,168,0,11);
String URLPrefix = "/shurf/public/device/put";
byte dnsserver[] = {8,8,8,8};
byte gateway [] = {192,168,0,1};
byte subnet [] = {255,255,255,0};
char server_address[] = "192.168.0.8";
EthernetClient client;
EthernetServer server(80);


void timer1_init (void) 
{
  TCCR1A=0x00;
  TCCR1B=0X05;
  TCNT1H=0xC2;
  TIMSK1-1<<OCIE1A;
  sei();
}

ISR(TIMER1_COMPA_vect)
{
  TCNT1H=0xC2;
  TCNT1L=0xF7;
  sec++;
  if(sec>59)
  {
    sec=0;
    minute++;
      if(minute>=60)
      {
        minute=0;
      }
  }
}  

void setup() {
  Serial.begin(9600);
  // --------------------------------------------
  // set Ethernet configuration
  Serial.println ("====Monitoring PV====");
  Serial.println ("Start Monitoring");
  Ethernet.begin(mac,ip,dnsserver,gateway,subnet); //inisiasi ethernet
  Serial.println ("IP Address   :");
  Serial.println (Ethernet.localIP());
  Serial.println (Ethernet.subnetMask());
  Serial.println (Ethernet.gatewayIP());
  Serial.println (Ethernet.dnsServerIP());
  server.begin();  
}

// baca input tegangan -------
float readVoltage()
{
  adc_tegangan = 0;
  time =  millis();  
  for (int i = 0; i <i_maks; i++)
  {
    if (time >= 2*60000)
    {
      adc_tegangan = 210;
      time = 0;
    } 
  tegangan = adc_tegangan * 20 / 255;
  total_voltage = total_voltage + tegangan;
  }

  voltage = 210; //total_voltage/i_maks;
  return voltage;
}  


// display time to serial -----
unsigned long print_time() {
  time = millis();
  Serial.print("Millis at ");
  Serial.print(time);
  Serial.println("");
  return time;
}

void show_time() {
  Serial.print("time :");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.print(seconds);
  Serial.println("");
  Serial.print("dates :");
  Serial.print(dates);
  Serial.print("/");
  Serial.print(months);
  Serial.print("/");
  Serial.print(years);
  Serial.println("");
  Serial.print("Voltage=");
  Serial.println(voltage);
  Serial.println("");
  delay(1000);
}

void loop(){
  // set voltage, current and power ----
  voltage = readVoltage(); 
//  current = readCurrent();
//  power = voltage*current;

  // -------------------------------------
  // read Real Time Clock
  RTC.readClock();
  hours   = RTC.getHours();
  minutes = RTC.getMinutes();
  seconds = RTC.getSeconds();
  dates   = RTC.getDate();
  months  = RTC.getMonth();
  years   = RTC.getYear();

/*  if ((time % 5000) == 0)
  {
    show_time();
  } */
  
// connect to server ------------
  do {
    delay(5000);
    Serial.print("Connecting.");
    while (!client.connected()) {
      client.connect (server_address,80);
      Serial.print(".");
    }
    Serial.println("");
    
    pushData(); // push data to server
    readData1(); // read feedback from server
    client.stop();
  } while ((print_time() % time2) == 0);
}

void pushData(){
  if (client.connected())
    {
      Serial.println("->Koneksi Sukses");
      // GET /shurf/public/add
      client.print ("GET " + URLPrefix + "put?");
      client.print("volt=");
      client.print(voltage);
    /*  client.print("&&");
      client.print("curr=");
      client.print(current);
      client.print("&&");
      client.print("powr=");      
      client.print(power);
      client.print("&&"); */
      client.print("minutes=");
      client.print(minutes);
      client.print("&&");
      client.print("hours=");
      client.print(hours);
      client.print("&&");
      client.print("dates=");
      client.print(dates);
      client.print("&&");
      client.print("months=");
      client.print(months);
      client.print("&&");
      client.print("years=");
      client.print(years);
      client.println( " HTTP/1.1");
      client.print ( "Host: ");
      client.println(server_address);
      client.println( "Connection: close" );
      client.println();
      client.println();
      delay(100);      
    }
    else
    {
      Serial.println ("--> Koneksi Gagal\n");
    }  
}

void readData1(){
  if (client.connected()){
    Serial.println("--Read feedback from server --");
    String packet;
    char c;
    Serial.println("Reading Data ... ");
    while(client.available()) {
      c = client.read();
      packet = packet + c;
    }
   //  
    Serial.println("Paket Respon:");
    Serial.println(packet);
    Serial.println("");
    Serial.println("");
  }
}


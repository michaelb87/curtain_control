#include <Time.h>
#include <NtpClientLib.h>
#include <TimeAlarms.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define ONBOARDLED 2

const uint8_t pin1 = 16; // D0
const uint8_t pin2 = 5; // D1
const uint8_t pin3 = 4; // D2
const uint8_t pin4 = 0; // D3
const uint8_t switchPin = 13; // D7
const uint8_t enableStepperPin = 12; // D6

const char* ssid = "campari";
const char* password = "aschach59";


ESP8266WebServer server(80);  

uint8_t sequence[] = { 0b1010, 0b0110, 0b0101, 0b1001 };
int16_t cur_pos = 0;
int16_t targ_pos = 0;
int16_t min_pos = 0;
int16_t max_pos = 50;
uint8_t state = 1; // 0 = no action, 1 = close, 2 = open
uint8_t direction = 0; // 0 = unknown, 1 = close, 2 = open

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

void setup() {
  hardwareInit();
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wifi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ntpInit();

  webServerInit();

  setTime(8,29,0,1,1,11); // set time to Saturday 8:29:00am Jan 1 2011

  Alarm.timerRepeat(15, Repeats);
  Alarm.alarmRepeat(19,11,0,Repeats);
  

}
void Repeats(){
  Serial.println("15 second timer");         
}
void ntpInit(){
  
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });
  
  NTP.begin("pool.ntp.org", 1, true);
  NTP.setInterval(63);
}

void hardwareInit(){
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(enableStepperPin, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  digitalWrite(enableStepperPin, LOW);
}

void webServerInit(){
    server.on("/", [](){
    String page = "<html><head><title>Curtain Control</title> <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/bulma/0.4.3/css/bulma.css\"> <script src=\"https://code.jquery.com/jquery-3.2.1.slim.min.js\" integrity=\"sha256-k2WSCIexGzOj3Euiig+TlR8gA0EmPjuc79OEeY5L45g=\" crossorigin=\"anonymous\"></script></head><body>";
    page += "<div class=\"container is-fluid\">";
    page += "<h1 class=\"title\">Curtain Control</h1> <h2 class=\"subtitle\"> Select an action</h2><hr>";
    page += "<div class=\"field is-grouped\">";
    page += "<a class=\"button\" href=\"/action?cmd=CLOSE\">Close</a>";
    page += "<a class=\"button\" href=\"/action?cmd=OPEN\">Open</a>";
    page += "<a class=\"button\" href=\"/action?cmd=STOP\">Stop</a>";
    page += "<a class=\"button\" href=\"/action?cmd=STATUS\">Status</a>";
    page += "<a class=\"button\" href=\"/action?cmd=CALIBRATE\">Calibrate</a>";
    page += "</div>"; // end field
    page += "</div>"; // end container
    page += "<script>";
    page += "$(document).ready(function(e) {console.log('loaded');});";
    page += "</script>";
    page += "</body></html>";
    
    server.send(200, "text/html", page);
  });

  server.on("/action", [](){
    String cmd = server.arg("cmd");
    cmd.toLowerCase();
    if(cmd == "open") {
      targ_pos= -32767;
      changeState(2);
      server.send(200, "application/json", "{\"error\": false, \"msg\" : \"ok\"}");
    } else if(cmd == "close") {
      targ_pos= 32767;
      changeState(1);
      server.send(200, "application/json", "{\"error\": false, \"msg\" : \"ok\"}");
    } else if( cmd.indexOf("set-") >= 0 ) {
      
      targ_pos = getPositionFromPercentage(cmd.substring(4).toInt());
      if(targ_pos<cur_pos)
        changeState(2);
      else if(targ_pos>cur_pos)
        changeState(1);
      server.send(200, "application/json", "{\"error\": true, \"msg\" : \"Switching to position = " +String(targ_pos) +"\" }");
    } else if(cmd == "status"){
        server.send(200, "application/json", "{\"error\": false, \"cur_position\" : " + String(cur_pos) + ", \"max_pos\": " + String(max_pos) + ", \"min_pos\" : " +min_pos + ", \"state\" : " +state + "}");
    } else if(cmd == "stop") {
      targ_pos= cur_pos;
      changeState(0);
      server.send(200, "application/json", "{\"error\": false, \"msg\" : \"ok\"}");
    } else if(cmd == "calibrate" ){
       targ_pos = 32767;
       server.send(200, "application/json", "{\"error\": false, \"msg\" : \"ok\"}");
    } else {
      server.send(200, "application/json", "{\"error\": true, \"msg\" : \"please supply ?cmd. Options are: OPEN, CLOSE, SET-50, STOP, STATUS, CALIBRATE \"}");
    }
   
  });

  server.begin();
  Serial.println("HTTP server started");
}


/* MAIN LOPP  */
void loop() {

  
  server.handleClient();
  handleSwitchInterrupt();
  if(cur_pos>max_pos)
    max_pos=cur_pos;
  if(cur_pos<min_pos)
    min_pos=cur_pos;
  if(min_pos >= max_pos)
    min_pos = max_pos-1;

  if (syncEventTriggered) {
    processSyncEvent(ntpEvent);    
    syncEventTriggered = false;
  }
  
  updateStepper();

  static int last = 0;

  if ((millis() - last) > 5100) {
    //Serial.println(millis() - last);
    last = millis();
    Serial.print(NTP.getTimeDateString()); Serial.print(" ");
    Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
    Serial.print("WiFi is ");
    Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
    Serial.print("Uptime: ");
    Serial.print(NTP.getUptimeString()); Serial.print(" since ");
    Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

  }
}

/* Updates stepper position*/
void updateStepper(){
  if(cur_pos != targ_pos && abs(cur_pos) != 32767) {
    digitalWrite(enableStepperPin, HIGH);
    
    if(cur_pos > targ_pos)
      cur_pos--;
    else cur_pos++;
    uint8_t v=sequence[uint16_t( (cur_pos ) )%4];
    digitalWrite(pin1, v>>0&1);
    digitalWrite(pin2, v>>1&1);
    digitalWrite(pin3, v>>2&1);
    digitalWrite(pin4, v>>3&1);
    delay(20);
    //Serial.println(cur_pos);
  } else {
    digitalWrite(enableStepperPin, LOW);
  }
  
}

void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  if (ntpEvent) {
    Serial.print("Time Sync error: ");
    if (ntpEvent == noResponse)
      Serial.println("NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("Invalid NTP server address");
  }
  else {
    Serial.print("Got NTP time: ");
    Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
    //String timesStr=NTP.getTimeStr();
    //setTime(NTP.getTime());
    
  }
}


void handleSwitchInterrupt(){
  if(digitalRead(switchPin) == 0) {
    if(! ( ( state==2 && cur_pos > min_pos) || ( state==1 && cur_pos < max_pos )  )) {
      targ_pos=cur_pos;
      if(direction == 1)
        max_pos=cur_pos;
      else if(direction == 2)
        min_pos=cur_pos;
    }
  } else {
    changeState(0);
  }

}

void changeState(int newState){
  if(state != newState)
    state = newState;
  if(newState != 0)
    direction = newState;
}

float getPositionFromPercentage(int percentage){
  uint16_t range = max_pos - min_pos;
  return (float) (min_pos + (float) range/100*percentage);
}


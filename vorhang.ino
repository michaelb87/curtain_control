#include <Time.h>
#include <NtpClientLib.h>
#include <TimeAlarms.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "websites.h"

#define ONBOARDLED 2

#define INT16T_MAX 32767
#define NTP_UPDATE_INTERVAL 600

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

os_timer_t stepperTimer;

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event
boolean tasksInitialized = false;

/* constructor declarations */
void ntpInit();
void hardwareInit();
void webServerInit();
void updateStepper(void *pArg);
void changeState(int newState);
void noop();
void handleSwitchInterrupt();
float getPositionFromPercentage(int percentage);
void handleNotFound();
void loadFromFlash();
void updatePositionBoundaries();
void processSyncEvent(NTPSyncEvent_t ntpEvent);
void initializeTasks();


void setup() {
  hardwareInit();
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("INFO: Connecting to Wifi.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("INFO: Connected to ");
  Serial.println(ssid);
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  os_timer_setfn(&stepperTimer, updateStepper, NULL);
  os_timer_arm(&stepperTimer, 10, true);

  ntpInit();

  webServerInit();


  Alarm.timerOnce(15, noop); //required to initialize our scheduler >_>
  

}

void task(){
  Serial.print("task received ");
  targ_pos = INT16T_MAX;
  Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
}
void ntpInit(){
  
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });
  
  NTP.begin("pool.ntp.org", 1, true);
  NTP.setInterval(60);
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

   /*
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
    } );*/

  server.on("/action", [](){
    String cmd = server.arg("cmd");
    cmd.toLowerCase();
    if(cmd == "open") {
      targ_pos= -INT16T_MAX;
      changeState(2);
      server.send(200, "application/json", "{\"error\": false, \"msg\" : \"ok\"}");
    } else if(cmd == "close") {
      targ_pos= INT16T_MAX;
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
       targ_pos = INT16T_MAX;
       server.send(200, "application/json", "{\"error\": false, \"msg\" : \"ok\"}");
    } else {
      server.send(200, "application/json", "{\"error\": true, \"msg\" : \"please supply ?cmd. Options are: OPEN, CLOSE, SET-50, STOP, STATUS, CALIBRATE \"}");
    }
   
  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("INFO: HTTP server started");
}


/* MAIN LOPP  */
void loop() {

  
  server.handleClient();
  handleSwitchInterrupt();

  updatePositionBoundaries();


  if (syncEventTriggered) {
    processSyncEvent(ntpEvent);    
    syncEventTriggered = false;
  }
  
  Alarm.delay(0); // tick
}

/* Updates stepper position*/
void updateStepper(void *pArg){
  if(cur_pos != targ_pos && abs(cur_pos) != INT16T_MAX) {
    digitalWrite(enableStepperPin, HIGH);
    
    if(cur_pos > targ_pos)
      cur_pos--;
    else cur_pos++;
    uint8_t v=sequence[uint16_t( (cur_pos ) )%4];
    digitalWrite(pin1, v>>0&1);
    digitalWrite(pin2, v>>1&1);
    digitalWrite(pin3, v>>2&1);
    digitalWrite(pin4, v>>3&1);
  } else {
    digitalWrite(enableStepperPin, LOW);
  }
  
}

/* NTP sync event received */
void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  if (ntpEvent) {
    if (ntpEvent == noResponse)
      Serial.println("ERROR: NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("ERROR: Invalid NTP server address");
  }
  else {
    setTime(NTP.getTime());
    if(!tasksInitialized)
      initializeTasks();
    Serial.println("INFO: NTP Sync sucessfull at: " +NTP.getTimeDateString(NTP.getLastNTPSync()));
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

void updatePositionBoundaries(){
  if(cur_pos>max_pos)
    max_pos=cur_pos;
  if(cur_pos<min_pos)
    min_pos=cur_pos;
  if(min_pos >= max_pos)
    min_pos = max_pos-1;
}

void initializeTasks(){
  Serial.println("INFO: Initializing tasks");
  //Alarm.alarmRepeat(21,55,0,task);
  tasksInitialized = true;
  NTP.setInterval(NTP_UPDATE_INTERVAL);
  
}

float getPositionFromPercentage(int percentage){
  uint16_t range = max_pos - min_pos;
  return (float) (min_pos + (float) range/100*percentage);
}

bool loadFromFlash(String path) {
  if(path.endsWith("/")) path += "index.html";

  int NumFiles = sizeof(files)/sizeof(struct t_websitefiles);

  for(int i=0; i<NumFiles; i++) {
    if(path.endsWith(String(files[i].filename))) {     
      _FLASH_ARRAY<uint8_t>* filecontent;
      String dataType = "text/plain";
      unsigned int len = 0;
      WiFiClient client = server.client();
     
      dataType = files[i].mime;
      len = files[i].len;
     
      server.setContentLength(len);
      server.send(200, files[i].mime, "");
     
      filecontent = (_FLASH_ARRAY<uint8_t>*)files[i].content;
     
      filecontent->open();
     
      client.write(*filecontent, 100);
      return true;
    }
  }
  
 
  return false;
}
void handleNotFound() {
 
  // try to find the file in the flash
  if(loadFromFlash(server.uri())) return;
 
  String message = "File Not Found\n\n";
  message += "URI..........: ";
  message += server.uri();
  message += "\nMethod.....: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments..: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += "\n";
  message += "FreeHeap.....: " + String(ESP.getFreeHeap()) + "\n";
  message += "ChipID.......: " + String(ESP.getChipId()) + "\n";
  message += "FlashChipId..: " + String(ESP.getFlashChipId()) + "\n";
  message += "FlashChipSize: " + String(ESP.getFlashChipSize()) + " bytes\n";
  message += "getCycleCount: " + String(ESP.getCycleCount()) + " Cycles\n";
  message += "Milliseconds.: " + String(millis()) + " Milliseconds\n";
  server.send(404, "text/plain", message);
}

void noop(){}
void(* resetFunc) (void) = 0;



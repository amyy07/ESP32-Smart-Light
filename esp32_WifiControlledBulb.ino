#include <WiFi.h>  
#include <WebServer.h> 

// Wifi credentials 
const char* ssid = "wifi name"; // put the name of your wifi here
const char* password = "password" ;

const uint8_t RELAY_PIN = 13; //relay pin

WebServer server(80);   //create web server


// Variables
bool lightState = false;
bool timerRunning = false;
unsigned long timerStartTime = 0;
unsigned long timerDuration = 0;

// turn ON light
void turnOn()
{
    lightState = true;
    digitalWrite(RELAY_PIN, HIGH);    //sets GPIO13 HIGH 

    timerRunning = false;   //cancels timer

    server.sendHeader("Location","/");
    server.send(303);  //redirect
}


// turn OFF
void turnOff()
{
    lightState = false;
    digitalWrite(RELAY_PIN, LOW);

    timerRunning = false;

    server.sendHeader("Location","/");
    server.send(303);
}


// Timer

void startTimer()
{
    if(server.hasArg("minutes"))
    {
        int minutes = server.arg("minutes").toInt();

        if(minutes > 0)
        {
            lightState = true;
            digitalWrite(RELAY_PIN,HIGH);

            timerDuration = (unsigned long)minutes * 60000UL;
            timerStartTime = millis();

            timerRunning = true;

            Serial.print("Timer Started: ");
            Serial.print(minutes);
            Serial.println(" minute(s)");
        }
    }

    server.sendHeader("Location","/");
    server.send(303);
}

// Webpage
void handleRoot()
{
    String page;

    page += "<!DOCTYPE html>";
    page += "<html>";
    page += "<head>";

    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";

    page += "<style>";
    
    page += "body{font-family:'Courier New';background:#FFC5B8;text-align:center;margin-top:40px;}";

    page += "button{width:180px;height:55px;font-size:20px;margin:8px;font-family:'Courier New', monospace;background-color:#FFEEEB;color:grey;border:none;}";

    page += "input{width:120px;height:35px;font-size:20px;text-align:center;font-family:'Courier New', monospace;}";

    page += "</style>";

    page += "</head>";
    page += "<body>";

    page += "<h1>ESP32 Smart Light</h1>";

    page += "<h2>Status: ";

    if(lightState)
        page += "ON";
    else
        page += "OFF";

    page += "</h2>";

    if(timerRunning)
    {
        unsigned long remaining =
        (timerDuration - (millis()-timerStartTime))/1000;

        page += "<h3>Timer Remaining: ";
        page += String(remaining);
        page += " seconds</h3>";
    }

    page += "<a href='/on'><button>Turn ON</button></a><br>";

    page += "<a href='/off'><button>Turn OFF</button></a>";

    page += "<hr>";

    page += "<h2>Timer</h2>";

    page += "<form action='/timer'>";

    page += "<input type='number' name='minutes' min='1' placeholder='Minutes'>";

    page += "<br><br>";

    page += "<button type='submit'>Start Timer</button>";

    page += "</form>";

    page += "</body>";
    page += "</html>";

    server.send(200,"text/html",page);
}


void setup()
{
    Serial.begin(115200);
    pinMode(RELAY_PIN,OUTPUT);
    digitalWrite(RELAY_PIN,LOW);
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid,password);

    while(WiFi.status()!=WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/",handleRoot);
    server.on("/on",turnOn);
    server.on("/off",turnOff);
    server.on("/timer",startTimer);
    server.begin();
    Serial.println("Web Server Started");
}



void loop()
{
    server.handleClient();

    if(timerRunning)
    {
        if(millis()-timerStartTime >= timerDuration)
        {
            timerRunning = false;
            lightState = false;
            digitalWrite(RELAY_PIN,LOW);
            Serial.println("Timer Finished");
        }
    }
}
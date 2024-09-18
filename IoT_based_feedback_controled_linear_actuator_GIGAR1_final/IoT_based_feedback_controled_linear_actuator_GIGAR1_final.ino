/*
Linear Actuator Configuration with actuator controller(L298N) and Arduino Giga R1(STM32H747xI) WiFi
Please concern about to connect an external antenna to micro UFL connector of Arduino Giga R1 otherwise WiFi does not work. 
(Ref. Arduino Giga Documentation=https://docs.arduino.cc/resources/pinouts/ABX00063-full-pinout.pdf)
--------------------------------------------
Linear Actuator-1:
--------------------------------------------
Enable Pin Connect-D2
Input Pin Connect (In1_Red Wire,In2_Black Wire)==(D3,D4)
Extend (D3,D4)==(HIGH,LOW)
Retract (D3,D4)==(LOW,HIGH)
--------------------------------------------
Linear Actuator-2:
--------------------------------------------
Enable Pin Connect-D5
Input Pin Connect (In1_Red Wire,In2_Black Wire)==(D6,D7)
Extend (D6,D7)==(HIGH,LOW)
Retract (D6,D7)==(LOW,HIGH)
--------------------------------------------
Linear Actuator-3:
--------------------------------------------
Enable Pin Connect-D8
Input Pin Connect (In1_Red Wire,In2_Black Wire)==(D9,D10)
Extend (D9,D10)==(HIGH,LOW)
Retract (D9,D10)==(LOW,HIGH)
--------------------------------------------
Linear Actuator-4:
--------------------------------------------
Enable Pin Connect-D11
Input Pin Connect (In1_Red Wire,In2_Black Wire)==(D12,D13)
Extend (D12,D13)==(HIGH,LOW)
Retract (D12,D13)==(LOW,HIGH)

*/


#include <SPI.h>
#include <WiFi.h> 

#define PORT 80                    //Webserver Port Number to set WiFi
#define potentiometer_pin_ac1 A0   //actuator 1 potentiometer analog pin
#define potentiometer_pin_ac2 A1   //actuator 2 potentiometer analog pin
#define potentiometer_pin_ac3 A2   //actuator 3 potentiometer analog pin
#define potentiometer_pin_ac4 A3   //actuator 4 potentiometer analog pin

// Update those values according to your Linear actuator properties (in millimeter)
#define STROKE_LENGTH      102  //Stroke length of linear actuator
#define POTENTIOMETER_MAX  2340 // Maximum reading value of potentiometer used in actuator
#define POTENTIOMETER_MIN  13   // Minimum reading value of potentiometer used in actuator
int TOLERANCE=5;                // Change Tollerance according to error of used linear actuator in millimeter

//Enable pins of L298N Motor controller 
int Lin_actuator_ENA[4] = {D2,D5,D8,D11};   //{EnablePin1,EnablePin2,EnablePin3,EnablePin4.}
//Input Pins of linear actuator 
int Lin_actuator_IN[8] = {D3,D4,D6,D7,D9,D10,D12,D13};   //{IN1_1,IN2_1,IN1_2,IN2_2.....etc.}
// Several status falg used for Linear actuator movement
bool actuator_status[8] = {false};  


//Put your WiFi gateway details
const char* ssid = "Dear SUHASHINI";  //WiFi SSID Name
const char* pass = "Suhash$2024$";    //WiFi Password

//WiFi PORT sending
WiFiServer server(PORT);
WiFiClient client;  // creating object for WiFiCLient

//Comment Out this Configuration if you want to fixed your IP adress according to your gateway
// IPAddress ip(192,168,178,110);     //Let assume your gateway IP is 192.168.X.Y then you can change only Y according to your needs and ensure that address is free from your router used IP adress list.
// IPAddress gateway(192,168,178,1);  // This is your router IP address reserved by itself which is also known as gateway ipaddress.
// IPAddress subnet(255,255,255,0);   //Do not need to do anything here. Remain as it is.

// Button and Box style sheet based on CSS----If you need to change your webserver design like, button color, font size, font style, button or box dimension etc.
String headAndTitle = "<head><style>"
                        ".button {"
                          "border: none;"
                          "color: white;"
                          "width: 300px;"
                          "padding: 10px;"
                          "text-align: center;"
                          "margin: 10px 50px;"
                        "}"
                        ".greenButton {background-color: green; font-size: 50px;}"
                        ".greenButton:hover {background-color: darkgreen; font-size: 50px;}"
                        ".redButton {background-color: red; font-size: 50px;}"
                        ".redButton:hover {background-color: darkred; font-size: 50px;}"
                        ".blueBox {"
                          "background-color: blue;"
                          "color: white;"
                          "width: 78%;" 
                          "padding: 10px;"
                          "text-align: center;"
                          "font-size: 40px;"
                          "font-family: arial;"
                          "margin: 10px 50px;"
                        "}"
                      "</style>"
                      "</head><meta http-equiv=\"refresh\" content=\"5\"></head>"
                      "</BR></BR><h1 align=\"center\">Control your xMCU by Browser</h1></div>"
                      "<div align=\"center\">Control Four Linear Actuators and get pot feedback values</BR></BR></div>";

//Button configuration for linear actuator 1
String Extend_ac1= "<a href=\"/Extend_on_1\"><button class=\"button greenButton\">Actuator_1 Extend</button></a>";
String extend_stop_1= "<a href=\"/ex_Stop_1\"><button class=\"button redButton\">Actuator_1 Extend Stop</button></a>";
String Retract_ac1= "<a href=\"/Retract_on_1\"><button class=\"button greenButton\">Actuator_1 Retract</button></a>";
String retract_stop_1= "<a href=\"/re_Stop_1\"><button class=\"button redButton\">Actuator_1 Retract Stop</button></a>";
//Button configuration for linear actuator 2
String Extend_ac2= "<a href=\"/Extend_on_2\"><button class=\"button greenButton\">Actuator_2 Extend</button></a>";
String extend_stop_2= "<a href=\"/ex_Stop_2\"><button class=\"button redButton\">Actuator_2 Extend Stop</button></a>";
String Retract_ac2= "<a href=\"/Retract_on_2\"><button class=\"button greenButton\">Actuator_2 Retract</button></a>";
String retract_stop_2= "<a href=\"/re_Stop_2\"><button class=\"button redButton\">Actuator_2 Retract Stop</button></a>";
//Button configuration for linear actuator 3
String Extend_ac3= "<a href=\"/Extend_on_3\"><button class=\"button greenButton\">Actuator_3 Extend</button></a>";
String extend_stop_3= "<a href=\"/ex_Stop_3\"><button class=\"button redButton\">Actuator_3 Extend Stop</button></a>";
String Retract_ac3= "<a href=\"/Retract_on_3\"><button class=\"button greenButton\">Actuator_3 Retract</button></a>";
String retract_stop_3= "<a href=\"/re_Stop_3\"><button class=\"button redButton\">Actuator_3 Retract Stop</button></a>";
//Button configuration for linear actuator 4
String Extend_ac4= "<a href=\"/Extend_on_4\"><button class=\"button greenButton\">Actuator_4 Extend</button></a>";
String extend_stop_4= "<a href=\"/ex_Stop_4\"><button class=\"button redButton\">Actuator_4 Extend Stop</button></a>";
String Retract_ac4= "<a href=\"/Retract_on_4\"><button class=\"button greenButton\">Actuator_4 Retract</button></a>";
String retract_stop_4= "<a href=\"/re_Stop_4\"><button class=\"button redButton\">Actuator_4 Retract Stop</button></a>";



void setup(){
  //Pin Setup for linear actuator 1 and the initial state of input
  pinMode(Lin_actuator_ENA[0], OUTPUT);
  digitalWrite(Lin_actuator_ENA[0], HIGH);
  pinMode(Lin_actuator_IN[0], OUTPUT);
  digitalWrite(Lin_actuator_IN[0], LOW);
  pinMode(Lin_actuator_IN[1], OUTPUT);
  digitalWrite(Lin_actuator_IN[1], LOW);
  //Pin Setup for linear actuator 2 and the initial state of input
  pinMode(Lin_actuator_ENA[1], OUTPUT);
  digitalWrite(Lin_actuator_ENA[1], HIGH);
  pinMode(Lin_actuator_IN[2], OUTPUT);
  digitalWrite(Lin_actuator_IN[2], LOW);
  pinMode(Lin_actuator_IN[3], OUTPUT);
  digitalWrite(Lin_actuator_IN[3], LOW);
  //Pin Setup for linear actuator 3 and the initial state of input
  pinMode(Lin_actuator_ENA[2], OUTPUT);
  digitalWrite(Lin_actuator_ENA[2], HIGH);
  pinMode(Lin_actuator_IN[4], OUTPUT);
  digitalWrite(Lin_actuator_IN[4], LOW);
  pinMode(Lin_actuator_IN[5], OUTPUT);
  digitalWrite(Lin_actuator_IN[5], LOW);
  //Pin Setup for linear actuator 4 and the initial state of input
  pinMode(Lin_actuator_ENA[3], OUTPUT);
  digitalWrite(Lin_actuator_ENA[3], HIGH);
  pinMode(Lin_actuator_IN[6], OUTPUT);
  digitalWrite(Lin_actuator_IN[6], LOW);
  pinMode(Lin_actuator_IN[7], OUTPUT);
  digitalWrite(Lin_actuator_IN[7], LOW);
  analogReadResolution(12);

  Serial.begin(115200);
  // while(!Serial) {}
  Serial.println("Minimal Program to Control Linear Actuator");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  // WiFi.config(ip, gateway, subnet);   //uncomment this If you want to fixed your IP address 
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500); 
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP-Address of xMCU module: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
  
void loop(){
  client = server.available();
  if(client){
    Serial.println("New client");
    boolean currentLineIsBlank = true;
    String clientMessage = "";
    
    while(client.connected()){     
      if(client.available()){
        char c = client.read();
        // Serial.write(c); //uncomment to print the request
        if(clientMessage.length()<50){
          clientMessage += c;
        }
        if(c == '\n' && currentLineIsBlank){
          evaluateClientMessage(clientMessage);
          break;
        }
        if(c == '\n'){
          currentLineIsBlank = true;
        } 
        else if(c != '\r'){
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("client disconnected");
  }
}

void evaluateClientMessage(String &msg){
   
  if(msg.indexOf("ex_Stop_1") > 0){
    actuator_status[0] = false;
  }
  else if(msg.indexOf("Extend_on_1") > 0){
    actuator_status[0] = true;
  }
  else if(msg.indexOf("re_Stop_1") > 0){
    actuator_status[1] = false;
  }
  else if(msg.indexOf("Retract_on_1") > 0){
    actuator_status[1] = true;
  }
  else if(msg.indexOf("ex_Stop_2") > 0){
    actuator_status[2] = false;
  }
  else if(msg.indexOf("Extend_on_2") > 0){
    actuator_status[2] = true;
  }
  else if(msg.indexOf("re_Stop_2") > 0){
    actuator_status[3] = false;
  }
  else if(msg.indexOf("Retract_on_2") > 0){
    actuator_status[3] = true;
  }
  else if(msg.indexOf("ex_Stop_3") > 0){
    actuator_status[4] = false;
  }
  else if(msg.indexOf("Extend_on_3") > 0){
    actuator_status[4] = true;
  }
  else if(msg.indexOf("re_Stop_3") > 0){
    actuator_status[5] = false;
  }
  else if(msg.indexOf("Retract_on_3") > 0){
    actuator_status[5] = true;
  }
  else if(msg.indexOf("ex_Stop_4") > 0){
    actuator_status[6] = false;
  }
  else if(msg.indexOf("Extend_on_4") > 0){
    actuator_status[6] = true;
  }
  else if(msg.indexOf("re_Stop_4") > 0){
    actuator_status[7] = false;
  }
  else if(msg.indexOf("Retract_on_4") > 0){
    actuator_status[7] = true;
  }
  

  for(int i=0; i<8; i++){
    digitalWrite(Lin_actuator_IN[i], actuator_status[i]);
  }

  String message = "";
  message += "<div align=\"center\";>";
  (actuator_status[0]==true)?(message += extend_stop_1):(message += Extend_ac1);
  (actuator_status[1]==true)?(message += retract_stop_1):(message += Retract_ac1);

  (actuator_status[2]==true)?(message += extend_stop_2):(message += Extend_ac2);
  (actuator_status[3]==true)?(message += retract_stop_2):(message += Retract_ac2);

  (actuator_status[4]==true)?(message += extend_stop_3):(message += Extend_ac3);
  (actuator_status[5]==true)?(message += retract_stop_3):(message += Retract_ac3);

  (actuator_status[6]==true)?(message += extend_stop_4):(message += Extend_ac4);
  (actuator_status[7]==true)?(message += retract_stop_4):(message += Retract_ac4);

  // Algorithom for mapping analog data of feedback Position given by potentiometer to the sepecific IP (webserver) 
  //Accuator-1
  float potentiometer_value_1 = analogRead(potentiometer_pin_ac1);
  float stroke_pos_1 = map(potentiometer_value_1, POTENTIOMETER_MIN, POTENTIOMETER_MAX, 0, STROKE_LENGTH);
  //Accuator-2
  float potentiometer_value_2 = analogRead(potentiometer_pin_ac2);
  float stroke_pos_2 = map(potentiometer_value_2, POTENTIOMETER_MIN, POTENTIOMETER_MAX, 0, STROKE_LENGTH);
  //Accuator-3
  float potentiometer_value_3 = analogRead(potentiometer_pin_ac3);
  float stroke_pos_3 = map(potentiometer_value_3, POTENTIOMETER_MIN, POTENTIOMETER_MAX, 0, STROKE_LENGTH);
  //Accuator-4
  float potentiometer_value_4 = analogRead(potentiometer_pin_ac4);
  float stroke_pos_4 = map(potentiometer_value_4, POTENTIOMETER_MIN, POTENTIOMETER_MAX, 0, STROKE_LENGTH);
    
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); 
  client.println(); // this line is not redundant!
  client.println(headAndTitle);
  client.println(message);
  //Sending Feedback position of linear actuator 1
  client.println("<div class=\"blueBox\">");
  client.println("Acuator_1 Position(+/-"+String(TOLERANCE)+"mm): ");
  client.println(String(stroke_pos_1,2));
  client.println("</div>");
  //Sending Feedback position of linear actuator 2
  client.println("<div class=\"blueBox\">");
  client.println("Acuator_2 Position(+/-"+ String(TOLERANCE) +"mm): ");
  client.println(String(stroke_pos_2, 2));
  client.println("</div>");
  //Sending Feedback position of linear actuator 3
  client.println("<div class=\"blueBox\">");
  client.println("Acuator_3 Position(+/-"+String(TOLERANCE)+"mm): ");
  client.println(String(stroke_pos_3, 2));
  client.println("</div>");
  //Sending Feedback position of linear actuator 4
  client.println("<div class=\"blueBox\">");
  client.println("Acuator_4 Position(+/-"+String(TOLERANCE)+"mm): ");
  client.println(String(stroke_pos_4, 2));
  client.println("&nbsp;</span>");
}

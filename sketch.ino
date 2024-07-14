#include <PubSubClient.h>//for MQTT 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <SPI.h>
#include <ESP32Servo.h>

// Definitions for time over wi-fi
#define NTP_SERVER "pool.ntp.org"
int UTC_OFFSET ; //UK time
#define UTC_OFFSET_DST 0

// OLED screen definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

//define buzzer,led and dht22
#define BUZZER 18
#define LED 15
#define DHT_PIN 12

//LDR and servo motor
#define right_LDR 36
#define left_LDR 39
#define SERVO 16

// define 4 push buttons
#define CANCEL 34
#define UP 35
#define DOWN 32
#define OK 33

//light intensity parameters
float GAMMA = 0.75;
float TEETA_OFFSET = 30; // minimum angle(default value=30)
float LIGHT_INTENSITY = 0; // change from 0-1

// Object Declarations
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

/////////////////////////////////////////////////////////////////////

//Wifi and mqtt clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

//store ldr and temperature data to send through mqtt
char temperature_array[8];
char right_LDR_array[5];
char left_LDR_array[5];

//servo motor initializing
int pos = 0;
Servo servo;

//////////////////////////////////////////////////////////////////////

//initialize time variables
int days=0;
int hours=0;
int minutes=0;
int seconds=0;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

// initialize alarm variables
bool alarm_enabled=true;
int n_alarms=3;
int alarm_hours[]={0,0,0};
int alarm_minutes[] = {0,0,0};
bool alarm_triggered[] = {false, false};

// buzzer sound notes
int n_notes=8;
int C=262;
int D=294;
int E=330;
int F=349;
int G=392;
int A=440;
int B=494;
int C_H=523;

int notes[]={C,D,E,F,G,A,B,C_H};

// declare modes for push button selections
int current_mode = 0;
int max_modes = 5;
String options[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - Set Alarm 3","5 - Disable Alarm"};

void setup() {
  // pin modes
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(right_LDR, INPUT);
  pinMode(left_LDR, INPUT);

  pinMode(CANCEL, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {} // Infinite loop if display initialization fails
  }

  display.display();
  delay(2000);

///////////////////////////////////////////////
  
  Mqtt_setup();
  servo.attach(16);

////////////////////////////////////////////////
  
  display.clearDisplay();
  print_line("Welcome to Medibox", 0, 0, 2);
  delay(3000);
  display.clearDisplay();

  
  // Connect to WiFi
  WiFi.begin("Wokwi-GUEST", ""); // Enter your SSID and password here
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting to WiFi", 0, 0, 2);
  }

  display.clearDisplay();
  print_line("Connected to WiFi", 0, 0, 2);
  configTime(UTC_OFFSET * 3600, UTC_OFFSET_DST, NTP_SERVER);

}

void loop() {

 ///////////////////////////////////////////////////////////////////// 
  //connect to mqtt broker
  if(!mqttClient.connected()){
    Serial.print("Reconnecting to MQTT Broker");
    connect_to_Broker();
  }
  mqttClient.loop();
  //publish topics with data
  updateLight();
  updateTemp();


  mqttClient.publish("LEFT_LIGHT-210418C", left_LDR_array);
  delay(50);
  mqttClient.publish("RIGHT_LIGHT-210418C", right_LDR_array);
  delay(50);
  mqttClient.publish("TEMPERATURE-210418C", temperature_array);
  delay(50);

////////////////////////////////////////////////////////////////////////

  // Handle sensor readings, display updates, user inputs, etc.
  update_time_with_check_alarm();
  if (digitalRead(OK) == LOW) {
    delay(2000);
    //Serial.println("Menu");
    go_to_menu();
  }

  check_temp();
  //checkSchedule();
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void Mqtt_setup() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}

void recieveCallback(char* topic, byte* payload, unsigned int length) {  //this is the main part that data collecting from the topic
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.print("\n");
 
  //receive minimum angle 
  if (strcmp(topic, "ANGLE-210418C") == 0){
    TEETA_OFFSET = atoi(payloadCharAr);
  }
  //receive control factor
  if (strcmp(topic, "CONTROLLING-FACTOR-210418C") == 0) {
    GAMMA = atof(payloadCharAr);
  }
  //receive main switch status
  if (strcmp(topic, "ON-OFF-210418C") == 0) {
    if (payloadCharAr[0] == '1') {
      digitalWrite(15, HIGH);
    } else {
      digitalWrite(15, LOW);
    }
  }
}

void connect_to_Broker() {
  while(!mqttClient.connected()){
    Serial.println("Attempting MQTT connection");
    if(mqttClient.connect("ESP32-210418C")){ //change
      Serial.println("MQTT connected");
      mqttClient.subscribe("ANGLE-210418C"); //change
      mqttClient.subscribe("CONTROLLING-FACTOR-210418C");
      mqttClient.subscribe("ON-OFF-210418C");
      mqttClient.subscribe("SCHEDULE-ON-210418C");
    }else{
      Serial.println("Failed connecting to MQTT");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void updateTemp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  String(data.temperature, 2).toCharArray(temperature_array, 8);
}

void updateLight() {

  float left_sv = analogRead(left_LDR) * 1.00;
  float right_sv = analogRead(right_LDR) * 1.00;

  float lsv_cha = (float)(left_sv - 4063.00) / (32.00 - 4063.00);
  float rsv_cha = (float)(right_sv - 4063.00) / (32.00 - 4063.00);
  //  Serial.println(String(lsv_cha)+" "+String(rsv_cha));

  updateAngle(lsv_cha, rsv_cha);

  String(lsv_cha).toCharArray(left_LDR_array, 5);
  String(rsv_cha).toCharArray(right_LDR_array, 5);
}

void updateAngle(float left_sv, float right_sv) {
  float max_I = left_sv;
  float D = 1.5;

  if (right_sv > max_I) {
    max_I = right_sv;
    D = 0.5;
  }

  int theta = TEETA_OFFSET * D + (180 - TEETA_OFFSET) * max_I * GAMMA;
  theta = min(theta, 180);

  servo.write(theta);
}

///////////////////////////////////////////////////////////////////////////////////////////

// print line function
void print_line(String text, int row, int column, int textSize) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();
}
// current time print function
void print_time_now(void) {
  display.clearDisplay();
  print_line(String(days)+ ":" + String(hours) + ":" + String(minutes) + ":" + String(seconds), 0, 0, 2);
  delay(500);
}
// function to automatically update the current time over WIFI
void update_time(void) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char day_str[3];
  char hour_str[3];
  char min_str[3];
  char sec_str[3];
  strftime(day_str, 3, "%d", &timeinfo);
  strftime(sec_str, 3, "%S", &timeinfo);
  strftime(hour_str, 3, "%H", &timeinfo);
  strftime(min_str, 3, "%M", &timeinfo);
  hours = atoi(hour_str);
  minutes = atoi(min_str);
  days = atoi(day_str);
  seconds = atoi(sec_str);
}
// function to update time with alarm checking
void update_time_with_check_alarm(void){

    display.clearDisplay();
    update_time();
    print_time_now();

    if (alarm_enabled) {

      for (int i = 0; i < n_alarms ; i++) {
        if (alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]) {
          ring_alarm(); 
          alarm_triggered[i] = true;
        

        }
      }
    }
}

// ring alarm function
void ring_alarm() {

  display.clearDisplay();
  print_line("Medicine Time",0, 0, 2);

  digitalWrite(LED, HIGH);

  bool break_happen=false;


  while(break_happen==false && digitalRead(CANCEL)==HIGH){
    for (int i = 0; i < n_alarms; i++) {
      if (digitalRead(CANCEL) == LOW){
        delay(200);
        break_happen=true;
        break;
      }
    

      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);

    }
  }
  
  digitalWrite(LED, LOW);
  display.clearDisplay();
}

// return values for button pressings
int wait_for_button_press() {
  while (true) {
    if (digitalRead(UP) == LOW) {
      delay(200);
      return UP;
    }

    else if (digitalRead(DOWN) == LOW) {
      delay(200);
      return DOWN;

    }

    else if (digitalRead(CANCEL) == LOW) {
      delay(200);
      return CANCEL;
    }

    else if (digitalRead(OK) == LOW) {
      delay(200);
      return OK;
    }
    update_time();

 }
}
// go to menu function
void go_to_menu(void) {
  while (digitalRead(CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(options[current_mode], 2, 0, 0);

    int pressed = wait_for_button_press();

    if (pressed == UP) {
      current_mode += 1;
      current_mode %= max_modes;
      delay(200);

    }

    else if (pressed == OK) {
      delay(200);
      run_mode(current_mode);
    }

    else if (pressed == DOWN) {
      delay(200);
      current_mode -= 1;
      if (current_mode < 0) {
      current_mode = max_modes - 1;
      }
    }
  }
}
// run mode function
void run_mode(int mode) {
  if (mode == 0) {
    set_time_zone();
  }
  else if (mode == 1 || mode == 2|| mode==3) {
    set_alarm(mode - 1);
  }
  else if (mode == 4) {
    alarm_enabled = false;

  }
}
void set_time_zone() {
  int temp_hour=0;
  while (true) {
    display.clearDisplay();
    print_line("Enter offset hour: " + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }

    else if (pressed == DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
      temp_hour = 23;
      }
    }

    else if (pressed == OK) {
      delay(200);
      hours = hours + temp_hour;
      break;
    }

    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  int temp_minute = 0;
  while (true) {
    display.clearDisplay();
    print_line("Enter offset minute: " + String(temp_minute), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }

    else if (pressed == DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }

    else if (pressed == OK) {
      delay(200);
      minutes = minutes + temp_minute;
      break;
    }

    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }
  UTC_OFFSET = temp_hour*3600 + temp_minute*60;
  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);

  display.clearDisplay();
  print_line("Time is set", 0, 0, 2);
  delay(1000);
}
void set_alarm(int alarm) {

  int temp_hour = alarm_hours[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }

    else if (pressed == DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
      temp_hour = 23;
      }
    }

    else if (pressed == OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }

    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  int temp_minute = alarm_minutes[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }

    else if (pressed == DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }

    else if (pressed == OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }

    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is set", 0, 0, 2);
  delay(1000);
}
void check_temp(void) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    String(data.temperature, 2).toCharArray(temperature_array, 8);

    if (data.temperature > 32) {
       // display.clearDisplay();
        print_line("TEMP HIGH", 40, 0, 1);
    } else if (data.temperature < 26) {
       // display.clearDisplay();
        print_line("TEMP LOW", 40, 0, 1);
    }

    if (data.humidity > 80) {
       // display.clearDisplay();
        print_line("HUMD HIGH", 50, 0, 1);
    } else if (data.humidity < 60) {
       // display.clearDisplay();
        print_line("HUMD LOW", 50, 0, 1);
    }  
    delay(1000); 
}



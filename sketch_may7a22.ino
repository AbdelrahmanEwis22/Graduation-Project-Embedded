#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Adafruit_MLX90614.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define HOST "myapis.whf.bz"
#define WIFI_SSID "7-1"                 //wifi name
#define WIFI_PASSWORD "G*01282835112#g" // wifi password
#define JWT_TOKEN "your_jwt_token_here" 

// Pin definitions
const int PulseWire = A0;   
const int LED13 = 13;          // The on-board Arduino LED
int Threshold = 550;     // Signal pin connected to analog pin A0

          // Determine which Signal to "count as a beat" and which to ignore

unsigned long lastBeatTime = 0;
unsigned long thisBeatTime = 0;
int BPM = 0;
double objectTemp=0;
bool beatDetected = false;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// GSM variables
char phone[] = "011xxxxxxxx"; // Specify the phone number
SoftwareSerial mySerial(D4, D3); // RX, TX
char strn[100], coord[50], lati_arr_1[25], lati_arr_2[25], long_arr_1[25], long_arr_2[25];
int c = 0, i = 0;
double Db_lati_1, Db_lati_2, Db_long_1, Db_long_2, Db_lati, Db_long;
String str_lati, str_long;
//API
void checkWiFiConnection() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to Wi-Fi...");
    delay(1000);
  }
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}
// End API
void setup() {
   pinMode(LED13, OUTPUT);
  Serial.begin(115200);
 // Set pulse pin as input
  mySerial.begin(115200);
    Serial.println("Communication Started\n");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  checkWiFiConnection();
   while (!Serial);

  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
}

void loop() {
  int sensorValue = analogRead(PulseWire);    // Read the sensor value

  if (sensorValue > Threshold) {
    thisBeatTime = millis();

    if (beatDetected == false) {
      beatDetected = true;
      BPM = 60000 / (thisBeatTime - lastBeatTime); // Calculate BPM
      lastBeatTime = thisBeatTime;
      digitalWrite(LED13, HIGH);   // Turn on LED
      // Print BPM value to serial monitor
      Serial.print("BPM: ");
      Serial.println(BPM +50);
     
       Serial.print("Ambient temperature = "); 
  Serial.print(mlx.readAmbientTempC());
  Serial.print("°C");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempC()); 
  Serial.println("°C");
  
float ambientTemp = mlx.readAmbientTempC();
   objectTemp = mlx.readObjectTempC();

  // Check if object temperature is above normal
  if (objectTemp > 37.5) { // You can adjust this threshold as needed
    sendLocation();
    delay(6000);
    checkWiFiConnection();
  }
   if (BPM < 60 || BPM > 100) { // If BPM is abnormal, send SMS with location
        // Retrieve current location
        sendLocation();
        delay(6000);
        checkWiFiConnection();
      }

  Serial.println("-----------------------------------------------------------------");
  delay(2000);
    }
  
  }else {
    beatDetected = false;
    digitalWrite(LED13, LOW);    // Turn off LED
  }
  delay(200);
}
void sendLocation() {
  // GPS Communication
  strn[9] = '\0';
  mySerial.println("AT");
  delay(200);
  for (i = 0; i < 99; i++) strn[i] = 0;
  c = 0;
  while (!mySerial.available());
  while (mySerial.available()) {
    strn[c] = (mySerial.read());
    if (c < 9) c++;
  }
  Serial.println("");
  c = 0;
  delay(2000);
  mySerial.println("AT+CGPSPWR=1");
  delay(200);
  for (i = 0; i < 99; i++) strn[i] = 0;
  c = 0;
  while (!mySerial.available());
  while (mySerial.available()) {
    strn[c] = (mySerial.read());
    c++;
  }
  mySerial.println("AT+CGPSRST=1");
  delay(200);
  delay(200);
  for (i = 0; i < 99; i++) strn[i] = 0;
  c = 0;
  while (!mySerial.available());
  while (mySerial.available()) {
    strn[c] = (mySerial.read());
    c++;
  }
  mySerial.println("AT+CGPSINF=0");
  delay(200);
  for (i = 0; i < 99; i++) strn[i] = 0;
  c = 0;
  int j = 0;
  while (!mySerial.available());
  while (mySerial.available()) {
    strn[c] = (mySerial.read());
    Serial.print(strn[c]);
    c++;
  }
  Serial.print("\n ");
  for (i = 27; i < 50; i++) {
    Serial.print(strn[i]);
    coord[i - 27] = strn[i];
  }
  Serial.print("\n ");
  delay(1000);
  for (i = 0; i < 23; i++) {
    if (i < 2) {
      lati_arr_1[i] = coord[i];
    } else if (i >= 2 && i <= 10) {
      lati_arr_2[i - 2] = coord[i];
    } else if (i >= 12 && i <= 13) {
      long_arr_1[i - 12] = coord[i];
    } else if (i >= 14 && i <= 22) {
      long_arr_2[i - 14] = coord[i];
    }
  }
  String lati_1(lati_arr_1);
  String lati_2(lati_arr_2);
  String long_1(long_arr_1);
  String long_2(long_arr_2);
  Db_lati_1 = lati_1.toDouble();
  Db_lati_2 = ((lati_2.toDouble()) / 60);
  Db_lati = Db_lati_1 + Db_lati_2;
  str_lati = String(Db_lati, 6);
  Db_long_1 = long_1.toDouble();
  Db_long_2 = ((long_2.toDouble()) / 60);
  Db_long = Db_long_1 + Db_long_2;
  str_long = String(Db_long, 6);
  String coord_string = String(str_lati) + ',' + String(str_long);
  String c = "https://www.google.com/maps/place/";
  String map_link = c + coord_string;


  // GSM Communication
  delay(1500);
  mySerial.println("AT");
  delay(1500);
  mySerial.println("AT+CSCS=\"GSM\"");
  delay(1500);
  mySerial.println("AT+CMGF=1");
  delay(1500);
  mySerial.print("AT+CMGS=\"");
  mySerial.print(phone);
  mySerial.println("\"");
  delay(150);
  mySerial.println("\n LOCATION IS " + coord_string + "\n" + map_link);
  mySerial.println((char)26);
  delay(100);
  Serial.println("Message is sent");

//API    
 void checkWiFiConnection();
  WiFiClient client;
  HTTPClient http;

  String postData = "{\"query\": \"mutation CreateDevice($macaddress: String!, $temperature: String!, $longitude: String!, $latitude: String!, $bloodpressure: String!) { createDevice(macaddress: $macaddress, temperature: $temperature, longitude: $longitude, latitude: $latitude, bloodpressure: $bloodpressure) { id macaddress temperature longitude latitude bloodpressure created_at updated_at }}\",\"variables\": { \"macaddress\": \"AB:CD:EF:01:23:45\", \"temperature\": \"" + String(objectTemp) + "\", \"longitude\": \"" + String(str_long) + "\", \"latitude\": \"" + String(str_lati) + "\", \"bloodpressure\": \"" + String(BPM+50) + "\"}}";

  Serial.print("Sending data: ");
  Serial.println(postData);

  http.begin(client, "http://" + String(HOST) + "/graphql");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(JWT_TOKEN)); // إضافة رأس المصادقة

  int httpCode = http.POST(postData);

  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println("Server response: " + response);
  } else {
    Serial.print("HTTP POST request failed with error code: ");
    Serial.println(httpCode);
    if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("Connection refused by the server.");
    } else if (httpCode == HTTP_CODE_NOT_FOUND) {
      Serial.println("Server resource not found.");
    } else {
      Serial.println("Unknown error occurred.");
    }
  }

  http.end();
  delay(5000);


}

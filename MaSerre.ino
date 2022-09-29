// initialize DHT inputs
#define DHTPIN 13
#define DHTTYPE DHT11
#include <DHT.h>;
DHT dht(DHTPIN, DHTTYPE);
float temperature;
float humidite;

//initialize light sensor
int capteur_lum = 4;
float luminosite;

//initialize ph sensor 
int PhSensorPin= 26;
float ph;

//initialize soil moisture
int pincapt = 12;
float humiditeDuSol;

// initialize ventilator
int ventilateur = 2;

// initialize ventilator
int ruban = 3;


#include <Firebase_ESP_Client.h>;
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#include <WiFi.h>;

// Insert your network credentials
#define WIFI_SSID "Verizon-SM-G930V-2380"
#define WIFI_PASSWORD "slater12"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
  
 
// Insert Firebase project API Key
#define API_KEY "AIzaSyB5z-jqlDXRBief_L4MeZc3iICRdZHozgw"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://test-arduino-2def4-default-rtdb.europe-west1.firebasedatabase.app"

  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ventilateur,OUTPUT); 
   pinMode(ruban,OUTPUT); 
   isConnected();
}

void loop(){
  // put your main code here, to run repeatedly:
   //collect sensor data

  //function to retrieve data from DHT sensors
  temperature = dht.readTemperature();
  humidite = dht.readHumidity();

  //function to retrieve data from PH sensors
  ph = getPhSol(PhSensorPin);

  // function to retrieve data from light sensor
    luminosite = getLuminosite(capteur_lum);
  
  // function to retrieve data from humidity sensor
    humiditeDuSol = getHumiditeSol(pincapt); 

   // function to send data to firebase
   sendData(temperature , humidite , luminosite , ph , humiditeDuSol);

   float etatRuban = getLuminositeFromFireBase();
   float etatVentilateur = getTemperatureFromFireBase();
   if(etatVentilateur == 1){
      digitalWrite(ventilateur , HIGH);
    }
   else{
     digitalWrite(ventilateur , LOW);
     }

    if(etatRuban == 1){
      digitalWrite(Ruban , HIGH);
    }
   else{
     digitalWrite(Ruban , LOW);
     }
}

//function to retrieve data from PH sensors
float getPhSol(int PhSensorPin){
  int measure = analogRead(PhSensorPin);
  double voltage = 3.3 / 4096.0 * measure;
  float Ph = 7 + ((2.5 - voltage)/ 0.18);
  return Ph;
}

// function to retrieve data from light sensor
float getLuminosite(int capteur_lum){
   float analog_lum = analogRead(capteur_lum);
   float val = ( (analog_lum/4096.00) * 100 );
  return val;
}

//function to retrieve data from soil moisture sensor
float getHumiditeSol(int pincapt ){
    float valeurcapt = analogRead(pincapt); // Lire la valeur du capteur
    float val = ( (valeurcapt/4096.00) * 100 );
    return val;
}

// function to turn on ventilator
void automaticVentilator(float temperature){
  if (temperature > 30){
    digitalWrite(ventilateur , HIGH);
    }
  }

//function to turn on led
void automaticLed(float luminosite){
  if (luminosite < 50){
    digitalWrite(ruban , HIGH);
    }
  }
//function to send data to firebase
void sendData(float temperature , float humidite , float luminosite , float ph , float humiditeDuSol){
   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path 
    if (Firebase.RTDB.setFloat(&fbdo, "Serre/temperature" , temperature)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

     if (Firebase.RTDB.setFloat(&fbdo, "Serre/humidite" , humidite)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Serre/luminosite" , luminosite){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else { 
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

     if (Firebase.RTDB.setFloat(&fbdo, "Serre/ph" , ph)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else { 
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
     if (Firebase.RTDB.setFloat(&fbdo, "Serre/humiditeDuSol " , humiditeDuSol)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else { 
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
  }
}


void isConnected(){
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
        Serial.println("ok");
        signupOK = true;
    }
    else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
        
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

int getHumiditeDuSolFromFireBase(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getInt(&fbdo, "/Serre/humidite_du_sol")) {
        int value = fbdo.intData();
        return value;
    }
    else{
      return 0;
    }
  }
  else{
    return 0;
    }
 }

 int getTemperatureFromFireBase(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getInt(&fbdo, "/Serre/temperature")) {
        int value = fbdo.intData();
        return value;
    }
    else{
      return 0;
    }
  }
  else{
    return 0;
    }
 }

int getLuminositeFromFireBase(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getInt(&fbdo, "/Serre/luminosite")) {
        int value = fbdo.intData();
        return value;
    }
    else{
      return 0;
    }
  }
  else{
    return 0;
    }
 }

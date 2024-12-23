#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h>

// WiFi parameters to be configured
//const char* ssid = "HUAWEI-B311-944D"; // Write here your router's username
//const char* password = "N1015HJ430H"; // Write here your router's passward

const char* ssid = "Vuk Karadzic"; // Write here your router's username
const char* password = "rDM2buha"; // Write here your router's passward

const char* serverName = "http://www.poljoskolabac.edu.rs/mb/zemlja_input.php";

String podaci;

void setup(void)
{ 
  pinMode(12, OUTPUT);     // izlaz za ledovku koja sija ako se poveze na WIFI
  digitalWrite(12, LOW);
  Serial.begin(9600);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);

  // vrti se u petlji i trepce led diodom dok se ne uspostavi WIFI veza
  while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(12, HIGH);
      delay(125);
      digitalWrite(12, LOW);
      delay(125);
      digitalWrite(12, HIGH);
      delay(125);
      digitalWrite(12, LOW);
      delay(125);
  }
  
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(12, HIGH);
  }
}

void loop() {
   // Provera WIFI veze
   // vrti se u petlji i trepce led diodom dok se ne uspostavi WIFI veza
  while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(12, HIGH);
      delay(125);
      digitalWrite(12, LOW);
      delay(125);
      digitalWrite(12, HIGH);
      delay(125);
      digitalWrite(12, LOW);
      delay(125);
  }

  // TODO: ovo preraditi sa led diodom koju ukuljucuje status promenljiva da nebi treptala prilikom svakog prolaska kroz petlju
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(12, HIGH);     
  }

  
  if(preuzmi_podatke()){
        sendPostRequest();
  }
    //delay(5000);
}

void sendPostRequest() {
  // Check if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    
    HTTPClient http;
    WiFiClient client; // Create WiFiClient object

    // Begin HTTP connection with WiFiClient and the server URL
    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // data to send
    //String podaci = "'77.7', '77.7', '777', '7.7', '777', '777', '777', CURRENT_TIMESTAMP, NULL";
    String obradjeni_podaci = urlEncode(podaci);
    String pocetak_upita = "podaci7u1=";
    String sql_upit = pocetak_upita += obradjeni_podaci;
    //String velicina_sql_upita = String(sizeof(sql_upit));
    
    // Set HTTP header
    //http.addHeader("Content-Type", "text/plain");
    //http.addHeader("Content-Length", velicina_sql_upita);
    //http.addHeader("accept", "*/*");
    //http.addHeader("user-agent", "curl/8.5.0");

    // Send POST request
    int httpResponseCode = http.POST(sql_upit);
    //String payload = http.getString();

    // Check for successful POST
    if (httpResponseCode > 0) {
//      Serial.print("POST Request sent successfully, Response Code: ");
//      Serial.println(httpResponseCode);
//      Serial.println("Payload");
//      Serial.println(payload);
    } else {
//      Serial.print("Error in sending POST request, Response Code: ");
//      Serial.println(httpResponseCode);
//      Serial.println("Payload");
//      Serial.println(payload);
    }

    // End HTTP connection
    http.end();
  } else {
    //Serial.println("WiFi not connected");
  }
}

// Define the urlEncode function at the top
String urlEncode(const String &str) {
  String encoded = "";
  char c;
  char code[3];
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      sprintf(code, "%%%02X", c);  // Convert character to hex code
      encoded += code;
    }
  }
  return encoded;
}

bool preuzmi_podatke(){
// Read and print response from ESP8266
  if (Serial.available()) {
    podaci = Serial.readString();
    return true;
  }
  return false;
}

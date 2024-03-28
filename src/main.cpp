/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp32/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>



const char* ssid = "XXXXX";  //ssid du réseau
const char* password = "XXXXX"; // mdp du réseau
String myMacAddress = "";
String server="http://xx.xx.xx.xx:xx/"; //adresse du serveur sur le reseau
String myDescription = "etage_1";



// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int id; // must be unique for each sender board
    int tempe;
    int humid;
    int occup;
    int lux;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;
struct_message board5;


bool sendHTTPReq = false;
char* node_mc_adress;

// Create an array with all the structures
struct_message boardsStruct[5] = {board1, board2, board3,board4,board5};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    char macStr[18];
    Serial.print("Packet received from: ");
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);
    node_mc_adress=macStr;
    Serial.printf("%s", node_mc_adress);
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
    // Update the structures with the new incoming data
    boardsStruct[myData.id-1].tempe = myData.tempe;
    boardsStruct[myData.id-1].humid = myData.humid;
    boardsStruct[myData.id-1].occup = myData.occup;
    boardsStruct[myData.id-1].lux= myData.lux;

    Serial.printf("temperature: %d \n", boardsStruct[myData.id-1].tempe);
    Serial.printf("humidite: %d \n", boardsStruct[myData.id-1].humid);
    Serial.printf("occupation: %d \n", boardsStruct[myData.id-1].occup);
    Serial.printf("lux: %d \n", boardsStruct[myData.id-1].lux);

    delay(500);
    sendHTTPReq= true;

    delay(500);

    Serial.println();
}

void setup() {
    //Initialize Serial Monitor
    Serial.begin(115200);

    delay(5000);

    //Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    myMacAddress=  WiFi.macAddress();
    String endpointtoConfigureSink = "http://192.168.10.140:3001/climatsense/iot/configureSink?mac_adress="+myMacAddress+"&description_sink="+myDescription;

    const char* endpointTOconfigureSinkInChar=endpointtoConfigureSink.c_str();

    //se connecter au réseau WI-FI
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(WiFi.status());

    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    //enregistrer le node dans la BDD
    if(WiFi.status()== WL_CONNECTED){
        WiFiClient client;
        HTTPClient http;

        // Your Domain name with URL path or IP address with path
        http.begin(client, endpointTOconfigureSinkInChar );

        // If you need Node-RED/server authentication, insert user and password below
        //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

        // Specify content-type header
        http.addHeader("accept", "*/*");
        // Data to send with HTTP POST// Send HTTP POST request
        int httpResponseCode = http.POST("");



        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        // Free resources
        http.end();
    }
    else {
        Serial.println("WiFi Disconnected");
    }



//Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(OnDataRecv);


}

void loop() {

    if (sendHTTPReq) {


        String endpointToSaveNodeData = server+"climatsense/iot/addData?mac_adress="+myMacAddress+"&description_node="+myData.id+"&mac_adress_node="+node_mc_adress+"&temperature_valeur_mesure="+myData.tempe+"&humidite_valeur_mesure="+myData.humid+"&occupation_valeur_mesure="+myData.occup+"&luminosite_valeur_mesure="+myData.lux;

        if(WiFi.status()== WL_CONNECTED){
            WiFiClient client;
            HTTPClient http;
            const char* endpointToSaveNodeDataChar=endpointToSaveNodeData.c_str();

            Serial.println(endpointToSaveNodeDataChar);

            // Your Domain name with URL path or IP address with path
            http.begin(client, endpointToSaveNodeDataChar );

            // If you need Node-RED/server authentication, insert user and password below
            //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

            // Specify content-type header
            http.addHeader("accept", "*/*");
            // Data to send with HTTP POST// Send HTTP POST request
            int httpResponseCode2 = http.POST("");



            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode2);

            // Free resources
            http.end();


            delay(500);
        }
        else {
            Serial.println("WiFi Disconnected");
        }

        sendHTTPReq= false;
    }

    // Acess the variables for each board
    int board1temp = boardsStruct[0].tempe;
    int board1humid = boardsStruct[0].humid;
    int board1occupation = boardsStruct[0].occup;
    int board1lux = boardsStruct[0].lux;




    delay(1000);


}
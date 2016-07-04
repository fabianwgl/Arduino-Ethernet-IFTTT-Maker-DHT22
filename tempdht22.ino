#include "DHT.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
 
#define DHTPIN 2     // Pin DHT 22 
#define DHTTYPE DHT22   // DHT 22  (AM2302)

byte mac[] = { 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };  // Cambiar por mac real

char MakerIFTTT_Key[] = "YOUR_KEY"; //  Clave o token que entrega Maker en IFTTT
char MakerIFTTT_Event[] = "YOUR_EVENT"; //  El evento creado en Maker IFTTT

#define READ_THIS_PIN       2      // Pin a leer
 
DHT dht(DHTPIN, DHTTYPE);

char *append_str(char *here, char *s) {
    while (*here++ = *s++)
  ;
    return here-1;
}

char *append_ul(char *here, unsigned long u) {
    char buf[20];      

    return append_str(here, ultoa(u, buf, 10));
}


//
// This is called once per iteration to read the pin
// and send a POST to trigger the IFTTT/Maker event
//

void update_event() {
    EthernetClient client = EthernetClient();

    // connect to the Maker event server
    client.connect("maker.ifttt.com", 80);

    // construct the POST request
    char post_rqst[256];    // hand-calculated to be big enough

    char *p = post_rqst;
    p = append_str(p, "POST /trigger/");
    p = append_str(p, MakerIFTTT_Event);
    p = append_str(p, "/with/key/");
    p = append_str(p, MakerIFTTT_Key);
    p = append_str(p, " HTTP/1.1\r\n");
    p = append_str(p, "Host: maker.ifttt.com\r\n");
    p = append_str(p, "Content-Type: application/json\r\n");
    p = append_str(p, "Content-Length: ");

    // we need to remember where the content length will go, which is:
    char *content_length_here = p;

    // it's always two digits, so reserve space for them (the NN)
    p = append_str(p, "NN\r\n");

    // end of headers
    p = append_str(p, "\r\n");

    // construct the JSON; remember where we started so we will know len
    char *json_start = p;
    float t = dht.readTemperature();
    // As described - this example reports a pin, uptime, and "hello world"
    p = append_str(p, "{\"value1\":\"");
    p = append_ul(p, t);
    p = append_str(p, "\",\"value2\":\"");
    p = append_ul(p, millis());
    p = append_str(p, "\",\"value3\":\"");
    p = append_str(p, "hello, world!");
    p = append_str(p, "\"}");

    // go back and fill in the JSON length
    // we just know this is at most 2 digits (and need to fill in both)
    int i = strlen(json_start);
    content_length_here[0] = '0' + (i/10);
    content_length_here[1] = '0' + (i%10);

    // finally we are ready to send the POST to the server!
    client.print(post_rqst);
    client.stop();
}
 
void setup() {
  Serial.begin(9600); 
  dht.begin();

   // this sets up the network connection, including IP addr via DHCP
    Ethernet.begin(mac);

    // the input pin for this example
    pinMode(READ_THIS_PIN, INPUT_PULLUP);
}

#define LOOP_DELAY_MSEC     (10800*1000L)   // 3 hours

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);
 
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.println(" *C ");

   // DHCP lease check/renewal (library only sends request if expired)
    Ethernet.maintain();

    // read the pins, send to IFTTT/Maker
    update_event();

    // only "this often"
    delay(LOOP_DELAY_MSEC);
 
}

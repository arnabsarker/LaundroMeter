#include < SPI.h >
#include < Ethernet.h >
#include < SoftwareSerial.h >
SoftwareSerial  BTSerial(2, 3); 
 
// MAC address, specific to the Ethernet shield we used
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x07, 0xCC };  
// IP address of our specific station
IPAddress ip(165, 123, 130, 208);
 
char server[] = "api.thingspeak.com";

//API key for the Thingspeak ThingHTTP already configured
const String apiKey = "OCRTSL6FYQAXW47D";

//the number the message should be sent to
const String sendNumber = "XXXXXXXXXXX";
 
EthernetClient client;
 
void setup() {
  //Begin communication with serial monitor and our other Arduino
  BTSerial.begin(9600);
  Serial.begin(9600);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  Ethernet.begin(mac, ip);
  delay(2000);
  Serial.println("Ready to send");
}

void loop() {
  if(BTSerial.available()){
    char inRead = (char) BTSerial.read();
    
    //Send a notification when the door is opened
    if(inRead == 'D'){
      Serial.println("Try to send door open text");
      sendSMS(sendNumber, "Your laundry door has been opened, hope that was you!");
    }
    
    //Send a notification when the machine turns off
    if(inRead == 'O'){
      Serial.println("Try to send machine off text");
      sendSMS(sendNumber, "Your laundry machine has turned off, come and get your clothes!");
    }
    
    //Send notification when machine turns on
    if(inRead == 'N'){
      Serial.println("Try to send machine on textl");
      sendSMS(sendNumber, "Looks like you've started a new load! We'll tell you when its done.");
    }
  } 
}

//Code for sending an SMS, mostly from instructables.com
void sendSMS(String number,String message)
{
  // Make a TCP connection to remote host
  if (client.connect(server, 80))
  {

    //should look like this...
    //api.thingspeak.com/apps/thinghttp/send_request?api_key={api key}&number={send to number}&message={text body}

    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(apiKey);
    client.print("&number=");
    client.print(number);
    client.print("&message=");
    client.print(message);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println(F("Connection failed"));
  } 

  // Check for a response from the server, and route it
  // out the serial port.
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }      
  }
  Serial.println();
  client.stop();
}

//Code used to encode messages so they actually send
String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg!='\0'){
    if( ('a' <= *msg && *msg <= 'z')
      || ('A' <= *msg && *msg <= 'Z')
      || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } 
    else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
} 

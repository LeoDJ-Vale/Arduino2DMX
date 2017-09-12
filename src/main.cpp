#include <Arduino.h>
//#include <DMXSerial.h>
#include <DmxSimple.h>

#include <ArtnetEthercard.h>
ArtnetEthercard artnet;
/*#include <Artnet.h>
#include <SPI.h>
Artnet artnet;*/

#define DEBUG true
//SoftwareSerial sSerial (4, 5); //RX, TX

const int universe = 1; //DMX universe to be translated
byte ip[] = {192, 168, 178, 97};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC}; //

void callback(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) 
{
    //have to use this workaround due to c++ limitations (no std::bind in arduino and can't call functions of objects from static function (no this pointer))
    artnet._udpCallback(dest_port, src_ip, src_port, data, len);
}

void setup() {
    //sSerial.begin(19200);
    //sSerial.println("\n\nArtnet2DMX");
    artnet.begin(mac, ip); //ip not needed for dhcp
    ether.udpServerListenOnPort(callback, ART_NET_PORT); //workaround


    //artnet.setCallback(artnet._udpCallback);
    //Serial.println(Ethernet.localIP()); //print dhcp IP

    //DMXSerial.init(DMXController);
    //DMXSerial.maxChannel(16);
    //DMXSerial.write(1, 255);
    DmxSimple.usePin(3);
    DmxSimple.maxChannel(512);
}
unsigned long lastPacket = 0;
bool blinkState = 0;
void loop() {
    if (artnet.read() == ART_DMX)
    {
        lastPacket = millis();
        if(artnet.getUniverse() == universe)
        {
            for (uint16_t i = 0 ; i < artnet.getLength() ; i++)
            {
              byte val = artnet.getDmxFrame()[i];
              //DMXSerial.write(i, val);
              DmxSimple.write(i, val);
            }
        }
    }

    if(millis() - lastPacket >= 1000) {
        lastPacket = millis();
        blinkState = !blinkState;
        //DMXSerial.write(1, blinkState ? 255 : 0);
        DmxSimple.write(1, blinkState ? 255 : 0);
    }
}
#include <Arduino.h>

#define DEBUG false

#include <DmxSimple.h>

#include <ArtnetEthercard.h>
ArtnetEthercard artnet;


const int universe = 1; //DMX universe to be translated
byte ip[] = {192, 168, 178, 97};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC}; //

void callback(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) 
{
    //have to use this workaround due to c++ limitations (no std::bind in arduino and can't call functions of objects from static function (no this pointer))

    #if DEBUG
        Serial.print("Raw UDP Packet: ");
        for(uint16_t i = 0; i < min(64, len); i++) {
            Serial.print((uint8_t)data[i]);
            Serial.print(" ");
        }
        Serial.println();
    #endif

    artnet._udpCallback(dest_port, src_ip, src_port, data, len);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nArtnet2DMX");
    artnet.begin(mac, ip); //ip not needed for dhcp
    ether.udpServerListenOnPort(callback, ART_NET_PORT); //workaround

    ether.printIp("My IP: ", ether.myip);
    
    DmxSimple.usePin(3);
    DmxSimple.maxChannel(512);
}
unsigned long lastPacket = 0, lastStatusPrint = 0;
bool blinkState = 0;
uint8_t packetCount = 0;
void loop() {
    if (artnet.read() == ART_DMX)
    {
        lastPacket = millis();
        if(artnet.getUniverse() == universe)
        {
            uint8_t *artnetFrame = artnet.getDmxFrame();
            packetCount++;

            for (uint16_t i = 0 ; i < artnet.getLength() ; i++)
            {
              byte val = artnetFrame[i];

              DmxSimple.write(i + 1, val);
              
              #if DEBUG
                if(i < 8) {
                    Serial.print(val);
                    Serial.print(" ");
                }
              #endif
            }
            #if DEBUG
                Serial.println();
            #endif
        }
        artnet.doneProcessing();
    }

    if(millis() - lastPacket >= 1000) {
        lastPacket = millis();
        blinkState = !blinkState;

        DmxSimple.write(1, blinkState ? 255 : 0);
        DmxSimple.write(5, blinkState ? 255 : 0);

        Serial.println(F("ERROR: No Art-Net Packets received for over 1 second."));
    }

    if(millis() - lastStatusPrint >= 1000) {
        Serial.print(((uint32_t)packetCount * 1000) / (millis() - lastStatusPrint));
        Serial.println(" packets / s");
        packetCount = 0;
        lastStatusPrint = millis();
    }
}
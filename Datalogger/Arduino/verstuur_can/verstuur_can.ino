#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
 
MCP2515 mcp2515(10);

const int buttonPin = 3;
int buttonState = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  
  while (!Serial);
  Serial.begin(9600);
  SPI.begin();               
 
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ); // zet misschien mcp op 16 mhz
  mcp2515.setNormalMode();  
  Serial.print("SPI and MCP ready"); 

}

void loop() {
    
    canMsg.can_id  = 0x036; // in dec is dit 54
    canMsg.can_dlc = 8;               //CAN data length as 8
    canMsg.data[0] = 1;               
    canMsg.data[1] = 2;            
    canMsg.data[2] = 3;            
    canMsg.data[3] = 4;
    canMsg.data[4] = 5;
    canMsg.data[5] = 6;
    canMsg.data[6] = 7;
    canMsg.data[7] = 8;
  
    mcp2515.sendMessage(&canMsg);     //Sends the CAN message
}

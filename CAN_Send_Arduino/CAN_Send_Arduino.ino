#include <SPI.h> // Library for using SPI Communication
#include <mcp2515.h> // Library for using CAN Communication
 
struct can_frame canMsg;
MCP2515 mcp2515(10);
 
void setup(){
  SPI.begin(); // Begins SPI communication
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
}
 
void loop(){
  canMsg.can_id = 0x778; // CAN id as 0x036
  canMsg.can_dlc = 8; // CAN data length as 8
  canMsg.data[0] = 12; // value in [0] (max=255)
  canMsg.data[1] = 20; // value in [1] (max=255)
  canMsg.data[2] = 25; // value in [2] (max=255)
  canMsg.data[3] = 0x00;  // Rest all with 0
  canMsg.data[4] = 0x00;
  canMsg.data[5] = 0x00;
  canMsg.data[6] = 0x00;
  canMsg.data[7] = 0x00;
  mcp2515.sendMessage(&canMsg); //Sends the CAN message
  delay(5000);
}

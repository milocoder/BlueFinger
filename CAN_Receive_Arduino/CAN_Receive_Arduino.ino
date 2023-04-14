//CAN Receiver Code (Arduino UNO):


#include <SPI.h>              //Library for using SPI Communication 
#include <mcp2515.h>          //Library for using CAN Communication

//const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;  

struct can_frame canMsg; 
MCP2515 mcp2515(10);                 // SPI CS Pin 10 

void setup() {

  Serial.begin(9600);                //Begins Serial Communication at 9600 baudrate 

  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode();                  //Sets CAN at normal mode

}
int a;
int b;
int c;
int d;
int e;
int f;
int g;
int h;

float komma;
float getal;

void loop() 

{
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {

     a = canMsg.data[0];         
     b = canMsg.data[1];       
     c = canMsg.data[2];         
     d = canMsg.data[3];   
     e = canMsg.data[4];         
     f = canMsg.data[5];   
     g = canMsg.data[6];         
     h = canMsg.data[7];   
    }
  /*  Serial.print("waarde a: "); Serial.print(a);
    Serial.print("\n");
    Serial.print("waarde b: "); Serial.print(b);
    Serial.print("\n");
   Serial.print("waarde c: "); Serial.print(c);
    Serial.print("\n");
    Serial.print("waarde d: "); Serial.print(d);
    Serial.print("\n");
    Serial.print("waarde e: "); Serial.print(e);
    Serial.print("\n");
    Serial.print("waarde f: "); Serial.print(f);
    Serial.print("\n");
    Serial.print("waarde g: "); Serial.print(g);
    Serial.print("\n");
    Serial.print("waarde h: "); Serial.print(h);
    Serial.print("\n");
*/
    
    Serial.print(a);
    Serial.print(",");
    Serial.println(b);
    delay(500);
  
}

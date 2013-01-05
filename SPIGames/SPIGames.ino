
// inslude the SPI library:
#include <SPI.h>

char buf[100];
volatile byte pos;
volatile boolean process_it;

void setup(){
  Serial.begin(57600); 
  //enable slave mode 
  pinMode(MISO,OUTPUT);
  SPCR |= _BV(SPE);
  
  //initialize buffer
  pos = 0;
  
  SPI.attachInterrupt();
  Serial.println("SPI Testing");
}

ISR(SPI_STC_vect)
{
  byte c = SPDR;
  if(pos<sizeof buf)
  {
    buf[pos++]=c;  
  }
  //echo incoming byte back out
  SPDR = c;
}

void loop(){
  SPDR=0;
  if(pos==8){
    for(int i=0;i<8;i++){
      Serial.print(buf[i],HEX);
      Serial.print(" ");
    }
    Serial.println();
    pos=0;
  }
}

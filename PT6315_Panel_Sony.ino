//Note: This is a not clean code, this is only a pratical test
//where we don't want use any library or dependence of other's code blocks!!!

//The idea is to make the code so basic that you can let it change and see what happens!

/* Port controls  (Platform dependent) */
//#define CS_LOW()  PORTB &= ~1     /* CS=low */
//#define CS_HIGH() PORTB |= 1      /* CS=high */
//#define LCD_Port PORTB

#define VFD_in 5// If 0 write LCD, if 1 read of LCD
#define VFD_clk 6 // if 0 is a command, if 1 is a data0
#define VFD_stb 7 // Must be pulsed to LCD fetch data of bus
#define VFD_on 12 // This pin active the TR: Q204 to switch the TR: Q203 to ON and power up the DC/DC converter present on board.

#define delay_tcsh _delay_us(16)

#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Arduino Uno is 2 & 3
volatile byte buttonReleased = false;

unsigned char char0, char1, char2;
bool var = false;
unsigned int numberOfGrids = 0b00001000;  //Here I define the global value to the number of digits used, is cmd1.

unsigned char numbers[30] = {
  //76543210....FEDCBC98.................
  0B00110001, 0B00100011, 0B00000000, //0
  0B00100000, 0B00000010, 0B00000000, //1
  0B11010001, 0B00100010, 0B00000000, //2
  0B11100001, 0B00100010, 0B00000000, //3
  0B11100000, 0B00000011, 0B00000000, //4
  0B11100001, 0B00100001, 0B00000000, //5
  0B11110001, 0B00100001, 0B00000000, //6
  0B00100000, 0B00100010, 0B00000000, //7
  0B11110001, 0B00100011, 0B00000000, //8
  0B11100000, 0B00100011, 0B00000000  //9
  };
unsigned char letters[75] = {
  //76543210....FEDCBC98.................
  0B11110000, 0B00100011, 0B00000000, //A
  0B10100101, 0B00101010, 0B00000000, //B
  0B00010001, 0B00100001, 0B00000000, //C
  0B00100101, 0B00101010, 0B00000000, //D
  0B11010001, 0B00100001, 0B00000000, //E
  0B11010000, 0B00100001, 0B00000000, //F
  0B10110001, 0B00100001, 0B00000000, //G
  0B11110000, 0B00000011, 0B00000000, //H
  0B00000101, 0B00101000, 0B00000000, //I
  0B00110001, 0B00000010, 0B00000000, //J
  0B01010010, 0B00000101, 0B00000000, //K
  0B00010001, 0B00000001, 0B00000000, //L
  0B00110000, 0B00010111, 0B00000000, //M
  0B00110010, 0B00010011, 0B00000000, //N
  0B00110001, 0B00100011, 0B00000000, //O
  0B11010000, 0B00100011, 0B00000000, //P
  0B00110011, 0B00100011, 0B00000000, //Q
  0B11010010, 0B00100011, 0B00000000, //R
  0B10100001, 0B00110000, 0B00000000, //S
  0B00000100, 0B00101000, 0B00000000, //T
  0B00110001, 0B00000011, 0B00000000, //U
  0B00011000, 0B00000101, 0B00000000, //V
  0B00001010, 0B00010100, 0B00000000, //X
  0B00001001, 0B00100100, 0B00000000, //Z
  0B00111010, 0B00000011, 0B00000000  //W
};
void AD16315_send_cmd(unsigned char a) {
  // This send use the strob... good for send commands, not burst of data.
  unsigned char chr;
  chr = a;

  PORTD = (0 << VFD_clk);
  delayMicroseconds(1);//Delay
  PORTD = (0 << VFD_in);
  delayMicroseconds(1);//Delay
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
        for (int i = 0; i < 8; i++) // 8 bit 0-7 // aqui inverti para ficar com 0x88 correccto
        {
          PIND = (1 << VFD_in);
          delayMicroseconds(1);
          PIND = (1 << VFD_clk); // O (^)XOR logo só muda se for diferente de "1 1" e "0 0"
          delayMicroseconds(1);
          PIND = (0 << VFD_clk);
          delayMicroseconds(1);
          PORTD = (0 << VFD_in);
          delayMicroseconds(1);
        }
  PORTD = (1 << VFD_stb) | (0 << VFD_clk) | (1 << VFD_in);
  delayMicroseconds(10);
}
void send_data(unsigned char a) {
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data = a;
  //This don't send the strobe signal, to be used in burst data send
  for (mask = 0B00000001; mask > 0; mask <<= 1) { //iterate through bit mask
    PIND = (0 << VFD_clk);
    if (data & mask) { // if bitwise AND resolves to true
      PORTD = (1 << VFD_in);
    }
    else { //if bitwise and resolves to false
      PORTD = (0 << VFD_in); // send 0
    }
    _delay_us(48); //delay
    PIND = (1 << VFD_clk);
    _delay_us(48); //delay
    PIND &= ~(0 << VFD_clk);
    _delay_us(48); //delay
  }
}
void sendVFD(unsigned char a) {
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask

  data = a;
  digitalWrite(VFD_stb, LOW);
  _delay_us(48);
  for (mask = 0B00000001; mask > 0; mask <<= 1) { //iterate through bit mask
    PIND = (0 << VFD_clk);
    if (data & mask) { // if bitwise AND resolves to true
      PORTD = (1 << VFD_in);
    }
    else { //if bitwise and resolves to false
      PORTD = (0 << VFD_in); // send 0
    }
    _delay_us(48); //delay
    PIND = (1 << VFD_clk);
    _delay_us(48); //delay
    PIND &= ~(0 << VFD_clk);
    _delay_us(48); //delay
  }
  digitalWrite(VFD_stb, HIGH);
  _delay_us(48);
}
void pt6315_init(void) {
  delay(300); //power_up delay
  // Write to memory display, increment address, normal operation
  sendVFD(0B01000000);//
  delay(2);
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  sendVFD(0B11000000);//
  delay(4);
  // Configure VFD display (grids) cmd1
  sendVFD(numberOfGrids);// Number of grids and segments to be used!
  delay(2);
  // Set DIMM/PWM to value cmd4
  sendVFD((0B10001000) | 7);//0 min - 7 max  )(0b01010000)
  delay(4);
}
void msg(){
  sendVFD(0B01000000); // cmd 2
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3 Each grid use 3 bytes here define the 1º
  //
  send_data(0b00000000);  send_data(0b00000000);  send_data(0b00000000);  // 17-24;   9-16;    1-8;
  send_data(0b00000000);  send_data(0b00000000);  send_data(0b00000000);  // 41-48;  33-40;   25-32;
  send_data(0b10001000);  send_data(0b10001111);  send_data(0b00000000);  // 65-72;  57-64;   49-56; //H
  send_data(0b00100100);  send_data(0b00100010);  send_data(0b00000001);  // 89-96;  81-88;   73-80; //I
  send_data(0b00000000);  send_data(0b00000000);  send_data(0b00000000);  //113-120; 105-112; 97-104; //Space
  send_data(0b00001100);  send_data(0b00001111);  send_data(0b00000000);  //137-144; 129-136; 121-128; //F
  send_data(0b10001100);  send_data(0b10001000);  send_data(0b00000001);  //161-168; 153-160; 145-152; //O
  send_data(0b00001000);  send_data(0b00001000);  send_data(0b00000001);  //185-192; 177-184; 169-176; //L
  send_data(0b01001000);  send_data(0b01001011);  send_data(0b00000000);  //209-216; 201-208; 193-200; //K
  send_data(0b00001100);  send_data(0b10000111);  send_data(0b00000001);  //233-240; 225-232; 217-224; //S
  send_data(0b00000000);  send_data(0b00000000);  send_data(0b00000000);  
  send_data(0b00000000);  send_data(0b00000000);  send_data(0b00000000);  

  digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(4);
      sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
      delayMicroseconds(4);
      sendVFD((0B10001000) | 7); //cmd 4
      delayMicroseconds(4);
}
void clearVFD(void) {
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0B01000000); // cmd 2
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3

  for (int i = 0; i < 36; i++) {
    send_data(0b00000000); // data
  }

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(4);
}
void allOn(void) {
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0b01000000); // cmd 2: Normal operation, incremente, write to display.
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3: Address RAM to start fill data.

  for (int i = 0; i < 36; i++) {
    send_data(0b11111111); // data
  }

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(4);
    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    delay(1);
    sendVFD((0b10001000) | 7); //cmd 4
    delay(1);
}
void allOff(void) {
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0B01000000); // cmd 2 //Write display mode.
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(4);
  send_data(0b11000000); //cmd 3 Address memory position.

  for (int i = 0; i < 36; i++) {
    send_data(0b00000000); // data
  }

  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(4);
  sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    delay(1);
    sendVFD((0B10001000) | 7); //cmd 4
    delay(1);
}
void allOnDigit(uint8_t memAddr) {
  /*
    Here I clean all registers
    Could be done only on the number of grid
    to be more fast. The 12 * 3 bytes = 36 registers
  */
  sendVFD(0B01000000); // cmd 2: Normal operation, incremente, write to display.
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(4);
  send_data((0b11000000) | memAddr); //cmd 3: Address RAM to start fill data.

    send_data(0b11111111); // data
    send_data(0b11111111); // data
    send_data(0b11111111); // data
  
  digitalWrite(VFD_stb, HIGH);
  delayMicroseconds(4);
}
//**************ZONE Functions to write HI FOLKS***********************//
void msgHiFolks(void){
  for(int i=4; i> 0; i--){
  clearVFD();
  delay(200);
  writeCharH();
  writeCharI();
  writeCharSpace();
  writeCharF();
  writeCharO();
  writeCharL();
  writeCharK();
  writeCharS();
  delayMicroseconds(1000);
  }
}
void writeCharH(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x1E); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b11110000); // data
    send_data(0b00000011); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharI(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x1B); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00000101); // data
    send_data(0b00101000); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharSpace(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x18); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00000000); // data
    send_data(0b00000000); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharF(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x15); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b11010000); // data
    send_data(0b00100001); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharO(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x12); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00110001); // data
    send_data(0b00100011); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharL(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x0F); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b00010001); // data
    send_data(0b00000001); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharK(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x0C); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b01010010); // data
    send_data(0b00000101); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
void writeCharS(void){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x09); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(0b11100001); // data
    send_data(0b00100001); // data
    send_data(0b00000000); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(200);
}
//************END ZONE Functions to write HI FOLKS********************//

void findSegments(void) {
  //This function when uncommented in the Loop, will allow the identification of the sequence of segments in digits.
  byte AA = 0x00;
  byte AB = 0x00;
  byte AC = 0x00;
  unsigned long n = 0b000000000000000000000001; //Here I define as a total do 16 segments by digit or memory position!
  unsigned int numberOfSeg = 0;
  
  for (uint8_t i = 0; i < 36; i = i + 3) {  // 12 Grids * 3 bytes, the 36 is max of digits dividing by 3 bytes
    n = 0b000000000000000000000001;// Length of 3 bytes (24 bit)
        for(int m = 0; m < 16; m++) {
        //This start the control of button to allow continue teste!
        //Uncomment this cycle to use the button connected to Arduino 
        //When commented this allow the running automatic to bright segments on this panel.
        //Of course to other panels you need adapt the numbers and length of digits and segments.
                            while(1){
                                  if(!buttonReleased){
                                    delay(200);
                                  }
                                  else{
                                    delay(15);
                                    buttonReleased = false;
                                    break;
                                    }
                              }
            
          sendVFD(0B01000000); // cmd 2
          digitalWrite(VFD_stb, LOW);
          delayMicroseconds(4);
          send_data((0b11000000) | i); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

        AA= (n & 0x000000ffUL) >>  0;
        AB= (n & 0x0000ff00UL) >>  8;
        AC= (n & 0x00ff0000UL) >> 16;
              
          send_data(AA); // data
          send_data(AB); // data
          send_data(AC); // data
              
            if (n < 32767; n <<= 1) { //iterate through bit mask  //(8388608) ( 16777216) 
              numberOfSeg++;   
            }
            else{
              n = 0b000000000000000000000001;// Length of 3 bytes (24 bit)
              numberOfSeg++; 
              clearVFD();  //Is important this clear here to keep the VFD clean after skip over the group to 3 bytes!
            }
          Serial.println(numberOfSeg, DEC);Serial.print("  :     ");
          Serial.print(AC, HEX); Serial.print(", "); Serial.print(AB, HEX); Serial.print(", "); Serial.print(AA, HEX); Serial.print(", Digit seg. nº: "); Serial.println(m, HEX);

          digitalWrite(VFD_stb, HIGH);
          delayMicroseconds(4);

          sendVFD(numberOfGrids); //cmd1 Here I define the 11 grids and 16 Segments
          sendVFD((0B10001000) | 7); //cmd 4
          delayMicroseconds(4);
          //Comment the next cycle for if use button to skip to next segments.
          //This cycle is only used to keep segment bright some time, before skip to next!
              // for(int s = 0; s < 100; s++){
              //   delay(2);
              // }
        }
  }
}
void showLetters(void){
  for(uint8_t i = 0; i < 75; i=i+3){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x1E); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(letters[i]); // data
    send_data(letters[i+1]); // data
    send_data(letters[i+2]); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(500);
  }
}
void showNumbers(void){
  for(uint8_t i = 0; i < 30; i=i+3){
    sendVFD(0B01000000); // cmd 2
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data((0b11000000) | 0x03); //cmd 3 Each grid use 3 bytes here define the 1º, 2º, 3º until max address

    send_data(numbers[i]); // data
    send_data(numbers[i+1]); // data
    send_data(numbers[i+2]); // data
          
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);

    sendVFD(numberOfGrids); // cmd1 Here I define the 8 grids and 20 Segments
    sendVFD((0B10001000) | 7); //cmd 4
    delayMicroseconds(4);
    delay(500);
  }
}
void writeLED(){
  for(uint8_t led=0; led<4; led++){
              digitalWrite(VFD_stb, LOW);
                delayMicroseconds(4);
                send_data(0b01000001); //Write port of LED (PT6315 have 4 LED pins)
                switch (led){
                  case 0: send_data(0b00001110); break; //LED's 0,1,2,3 is reverse mode 0=On 1=Off, but in this panel they use a transistor which result as invertion!
                  case 1: send_data(0b00001101); break;
                  case 2: send_data(0b00001011); break;
                  case 3: send_data(0b00000111); break;
                default: send_data(0x00); break;
                }
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);
    //Serial.println(led, DEC); //This is only a print line to debug. Comment it.
    delay(500); //This is the time which LED will stay ON.
  }
}
void offLED(){
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(4);
    send_data(0b01000001); //Write port of LED (PT6315 have 4 LED pins)
    send_data(0b00000000);
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(4);
    //Serial.println(led, DEC); //This is only a print line to debug. Comment it.
    delay(500); //This is the time which LED will stay ON.
}
void setup() {
  cli();
  // put your setup code here, to run once:
  pinMode(VFD_stb, OUTPUT);// Must be pulsed to LCD fetch data of bus
  pinMode(VFD_in, OUTPUT);//  If 0 write LCD, if 1 read of LCD
  pinMode(VFD_clk, OUTPUT);// if 0 is a command, if 1 is a data0
  pinMode(VFD_on, OUTPUT);// if 0 is a command, if 1 is a data0

  digitalWrite(VFD_on, HIGH);  //Need be high because the transistor which active DC/DC converter on panel is a NPN
  
  pinMode( 13, OUTPUT); //Pin of pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  unsigned char a, b;

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                  buttonReleasedInterrupt,
                  FALLING);
  /*
    TCCR1B = (1 << CS12 | 1 << WGM12);
    OCR1A = 32768 - 1; //32768-1;
    TIMSK1 = 1 << OCIE1A;
  */
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  a = 0x33;
  b = 0x01;
  //You write a 1 to make a pin an output and a 0 to make it an input.
  CLKPR = (0x80);
  //Set PORT
  // DDRD = 0xFB;
  // PORTD = 0x00;
  //  DDRB = 0xFF;
  //  PORTB = 0x00;

  pt6315_init();
  clearVFD();
  
 sei();
}
void loop() {
  allOn();
  delay(2000);
      for(uint8_t n = 0; n < 2; n++){
          allOff();
          delay(500);
              for(uint8_t i = 0; i < 36; i=i+3){
                allOff();
                delay(500);
                allOnDigit(i);
                delay(500);
              }
          delay(1000);
          allOn();
          delay(500);
      }
      clearVFD();
      msgHiFolks();
      delay(500);
      clearVFD();
      showNumbers();
      clearVFD();
      showLetters();
      clearVFD();
//
    writeLED();
    offLED();
    //findSegments();  //Uncomment this line to find the segments order in digits!
}
void buttonReleasedInterrupt() {
  buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}

ISR(TIMER1_COMPA_vect) {  //Interrupt Trigger
  var = digitalRead(13);
  digitalWrite(13, !var);
  //actSSMMHH();
}

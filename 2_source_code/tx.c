

unsigned short temp;


//sbit RF_IRQ_TRIS at TRISB2_bit; //RB0, RB1SPI
sbit RF_CS_TRIS at TRISC3_bit;
sbit RF_CE_TRIS at TRISC4_bit;     //2
sbit SCK_TRIS at TRISC6_bit;
sbit SDI_TRIS at TRISC7_bit;      //  c7
sbit SDO_TRIS at TRISB7_bit;

sbit BUT_9_TRIS at TRISA5_bit;
sbit BUT_8_TRIS at TRISA4_bit;
sbit BUT_7_TRIS at TRISC5_bit;
sbit BUT_6_TRIS at TRISB6_bit;
sbit BUT_5_TRIS at TRISB5_bit;
sbit BUT_4_TRIS at TRISB4_bit;
sbit BUT_3_TRIS at TRISC2_bit;
sbit BUT_2_TRIS at TRISC1_bit;
sbit BUT_1_TRIS at TRISC0_bit;

//sbit IRQ_pin at RB2_bit; // RB0, RB1 SPI
sbit SS_pin at RC3_bit;
sbit CE_pin at RC4_bit;       //2
sbit SCK_pin at RC6_bit;
sbit MOSI_pin at RC7_bit; //SDO     c7
sbit MISO_pin at RB7_bit; //SDI 

sbit BUT_9_pin at RA5_bit;
sbit BUT_8_pin at RA4_bit;
sbit BUT_7_pin at RC5_bit;
sbit BUT_6_pin at RB6_bit;
sbit BUT_5_pin at RB5_bit;
sbit BUT_4_pin at RB4_bit;
sbit BUT_3_pin at RC2_bit;
sbit BUT_2_pin at RC1_bit;
sbit BUT_1_pin at RC0_bit;



#define R_REGISTER_cmd                0x00
#define W_REGISTER_cmd                0x20
#define R_RX_PL_WID_cmd               0x60
#define R_RX_PAYLOAD_cmd              0x61
#define W_TX_PAYLOAD_cmd              0xA0
#define W_ACK_PAYLOAD_cmd             0xA8
#define W_TX_PAYLOAD_NO_ACK_cmd       0xB0
#define FLUSH_TX_cmd                  0xE1
#define FLUSH_RX_cmd                  0xE2
#define REUSE_TX_PL_cmd               0xE3
#define NOP_cmd                       0xFF

#define CONFIG_reg                    0x00
#define EN_AA_reg                     0x01
#define EN_RXADDR_reg                 0x02
#define SETUP_AW_reg                  0x03
#define SETUP_RETR_reg                0x04
#define RF_CH_reg                     0x05
#define RF_SETUP_reg                  0x06
#define STATUS_reg                    0x07
#define OBSERVE_TX_reg                0x08
#define RPD_reg                       0x09
#define RX_ADDR_P0_reg                0x0A
#define RX_ADDR_P1_reg                0x0B
#define RX_ADDR_P2_reg                0x0C
#define RX_ADDR_P3_reg                0x0D
#define RX_ADDR_P4_reg                0x0E
#define RX_ADDR_P5_reg                0x0F
#define TX_ADDR_reg                   0x10
#define RX_PW_P0_reg                  0x11
#define RX_PW_P1_reg                  0x12
#define RX_PW_P2_reg                  0x13
#define RX_PW_P3_reg                  0x14
#define RX_PW_P4_reg                  0x15
#define RX_PW_P5_reg                  0x16
#define FIFO_STATUS_reg               0x17
#define DYNPD_reg                     0x1C
#define FEATURE_reg                   0x1D


unsigned char x = 0;
unsigned long  count_sleep_pause = 0;
unsigned long  count_led_pulse = 0;
unsigned long count_ = 0;

unsigned char nRF24L01_read(){
  unsigned char s = 0;
  unsigned char msg = 0;

  for(s = 0; s < 8; s++)
  {
      msg <<= 1;
      SCK_pin = 1;
      delay_us(8);
      if(MISO_pin != 0)
      {
        msg |= 1;
      }
      SCK_pin = 0;
      delay_us(8);
  }

  return msg;
}


void nRF24L01_write(unsigned char d){
  unsigned char s = 0;

  for(s = 0; s < 8; s++)
  {
    if((d & 0x80) != 0)
    {
      MOSI_pin = 1;
    }
    else
    {
      MOSI_pin = 0;
    }
    d <<= 1;
    SCK_pin = 1;
    delay_us(8);
    SCK_pin = 0;
    delay_us(8);
  }
}


void register_write(unsigned char reg, unsigned char value){
  SS_pin = 0;
  nRF24L01_write((reg | W_REGISTER_cmd));
  nRF24L01_write(value);
  SS_pin = 1;
  delay_us(8);
}


void write_command(unsigned char cmd){
  SS_pin = 0;
  nRF24L01_write(cmd);
  SS_pin = 1;
  delay_us(8);
}


unsigned char register_read(unsigned char reg){
  unsigned char value = 0;

  SS_pin = 0;
  nRF24L01_write((reg | R_REGISTER_cmd));
  value = nRF24L01_read();
  SS_pin = 1;
  delay_us(8);

  return value;
}


void set_TX_RX_address(unsigned char *addr, unsigned char bytes, unsigned char reg){
  unsigned char n = 0;

  SS_pin = 0;
  nRF24L01_write((reg | W_REGISTER_cmd));
  for(n = 0; n < bytes; n++)
  {
    nRF24L01_write(addr[n]);
  }
  SS_pin = 1;
  delay_us(8);
}


void flush_TX_RX(){
  register_write(STATUS_reg, 0x70);
  write_command(FLUSH_TX_cmd);
  write_command(FLUSH_RX_cmd);
}


void send_data(unsigned char bytes, unsigned char *value){
  unsigned char s = 0;

  flush_TX_RX();
  register_write(CONFIG_reg, 0x3A);

  SS_pin = 0;
  nRF24L01_write(W_TX_PAYLOAD_cmd);
  for(s = 0; s < bytes; s++)
  {
    nRF24L01_write(value[s]);
  }
  SS_pin = 1;
  delay_us(8);

  CE_pin = 1;
  delay_us(60);
  CE_pin = 0;

  register_write(CONFIG_reg, 0x38); // RX 
}

void receive_data(unsigned char bytes, unsigned char *value){
  unsigned char s = 0;

  SS_pin = 0;
  nRF24L01_write(R_RX_PAYLOAD_cmd);
  for (s = 0; s < bytes; s++)
  {
    value[s] = nRF24L01_read();
  }
  SS_pin = 1;
  delay_us(8);
}


void nrF24L01_init_TX(){
  unsigned char address[5] = {0x99, 0x99, 0x99, 0x99, 0x99};

  CE_pin = 0;

  register_write(SETUP_RETR_reg, 0x00);
  register_write(SETUP_AW_reg, 0x03);
  register_write(RF_SETUP_reg, 0x0E);      //0E-2Mbs 26-250Kbs  06-1Mbs
  register_write(RF_CH_reg, 25);         //09   номер канала
  register_write(EN_AA_reg, 0x00);
  register_write(CONFIG_reg, 0x38);
  set_TX_RX_address(address, 5, TX_ADDR_reg);
  set_TX_RX_address(address, 5, RX_ADDR_P0_reg);
  flush_TX_RX();

  CE_pin = 1;
}

void nrF24L01_init_RX(){
  unsigned char address[5] = {0x99, 0x99, 0x99, 0x99, 0x99};

  CE_pin = 0;

  register_write(CONFIG_reg, 0x38);
  register_write(SETUP_RETR_reg, 0x00);
  register_write(SETUP_AW_reg, 0x03);
  register_write(RF_SETUP_reg, 0x0E);      //0E-2Mbs 26-250Kbs  06-1Mbs
  register_write(RF_CH_reg, 25);         //09   номер канала
  register_write(EN_AA_reg, 0x00);
  register_write(RX_PW_P0_reg, 0x01);
  register_write(CONFIG_reg, 0x3B);
  set_TX_RX_address(address, 5, TX_ADDR_reg);
  set_TX_RX_address(address, 5, RX_ADDR_P0_reg);
  flush_TX_RX();

  CE_pin = 1;
}

unsigned char get_Status_Reg(){
  return register_read(STATUS_reg);
}



unsigned char val = 0;

void interrupt(){   // вектор прерывания, обработка
  //INTCON = 0;

   GIE_GIEH_bit = 0;  // глобальное разрешение прерывания
  // INT0IF_bit = 0;  // очистить флаг прерывания INT0
   INT1IF_bit = 0;  // очистить флаг прерывания INT1

  // INT0IE_bit = 0; //1-разрешить прерывание на INT0
   INT1IE_bit = 0; //1-разрешить прерывание на INT1
  // IDLEN_bit = 0;   // разрешить уходить в сон  0

   return;

 }


 void sleep_deep(){
     IPEN_bit = 0; // разрешить все немаскированные прерывания всех приоритетов
     GIE_GIEH_bit = 1;  // глобальное разрешение прерывания
     
    // INT0IF_bit = 0;  // очистить флаг прерывания INT0
     INT1IF_bit = 0;  // очистить флаг прерывания INT1
    // INT0IE_bit = 1; //разрешить прерывание на INT0
     INT1IE_bit = 1; //1-разрешить прерывание на INT1
     IDLEN_bit = 0;   // разрешить уходить в сон
     
     REFCON0 = 0X00;
     REFCON0 = 0X00;
     
      
        /*PORTA = 0X00;
        PORTB = 0X00;
        PORTC = 0x00;*/
      
        C1ON_bit = 0;     // отключить компаратор
        C2ON_bit = 0;
        ADCON1 = 0x0F;
        ANSEL = 0x00;
        ANSELH = 0x00;

        CM1CON0 = 0x00;     //  отключить компаратор 
        CM2CON1 = 0x00;
        
        ADCON0 = 0x00;
        ADCON1 = 0x00;
        
        TRISA     =  0xFF;
        TRISB     =  0xFF;
        TRISC     =  0xFF; //0b00000001;
        
     //   PORTA = 0X00;
     //   PORTB = 0X00;
     //   PORTC = 0x00;

    // SBOREN_bit = 1;
     /*OSCCON =  0b00000010;
     OSCCON2 = 0b00000011;
     OSCTUNE = 0b00000000;*/
     
    /*OSCCON=0;
    UPUEN_bit =0;
    ADON_bit=0;
    GIEH_bit=0;
    GIEL_bit=0;*/
     
     
     Delay_ms(1);
     asm{SLEEP};      // уйти в сон
     asm{nop};      // уйти в сон
    

     
   //  TRISA     =  0b11011111;
   //  TRISB     =  0b11110000;
   //  TRISC     =  0b00000111;

 }


void main(){



//delay_ms(500);

 //TRISA     =  0x00;
 //TRISB     =  0x00;
 //TRISC     =  0x00;
// ADCON1 = 0x0F;
// PORTA = 0X00;
// PORTB = 0X00;
// PORTC = 0x00;

 //RF_IRQ_TRIS = 1;
 RF_CS_TRIS = 0;
 RF_CE_TRIS = 0;
 SCK_TRIS = 0;
 SDI_TRIS = 0;  //MISO
 SDO_TRIS = 1;  //MOSI

 CE_pin = 0;
 SS_pin = 0;
 SCK_pin = 0;
 MOSI_pin = 0;
 
 //delay_ms(500);

 //nrF24L01_init_RX();
 nrF24L01_init_TX();

 //delay_ms(900);


 // ANSEL  = 0x04;              // AN2 pin аналоговый
 // ANSELH = 0;                 // другие AN pins = I/O
  C1ON_bit = 0;               // компаратор отключить
  C2ON_bit = 0;
  ADCON1 = 0x0F;
  ANSEL = 0x00;
  ANSELH = 0x00;
  
  CM1CON0 = 0x00;     // компаратор отключить
  CM2CON1 = 0x00;
  ADCON0 = 0x00;
  ADCON1 = 0x00;

 //BUT_1_TRIS = 1; // настроить как вход
 //LED_TRIS = 0; // настроить как выход
 
 BUT_9_TRIS =1;
 BUT_8_TRIS =1;
 BUT_7_TRIS =1;
 BUT_6_TRIS =1;
 BUT_5_TRIS =1;
 BUT_4_TRIS =1;
 BUT_3_TRIS =1;
 BUT_2_TRIS =1;
 BUT_1_TRIS =1;
 
 x = 1;
 
 start_1:
 
while (1){     // временно проверка

     
/*if (BUT_1_pin == 1) {
       LED_pin = 1;
       delay_ms(2000);
       LED_pin = 0;
       delay_ms(2000);
     }
    if (BUT_2_pin == 1) {
       LED_pin = 1;
       delay_ms(500);
      LED_pin = 0;
       delay_ms(500);
     }

  }*/
  
  
 //  LED_pin = 1;
 //    delay_ms(500);
 
// sleep_deep();
//IPEN_bit = 0; // разрешить все немаскированные прерывания всех приоритетов
// GIE_GIEH_bit = 1;  // глобальное разрешение прерывания
// INT0IF_bit = 0;  // очистить флаг прерывания INT0


 //   INT0IE_bit = 0; //разрешить прерывание на INT0
  //  IDLEN_bit = 0;   // разрешить уходить в сон
 //   Delay_us(200);
 //   asm{SLEEP}      // уйти в сон*/
    
    /*if (BUT_1_pin == 0) {
     x = 10;
     send_data(1, &x);
     delay_ms(500);
    }*/
   // LED_pin = 1;
   //  delay_ms(500);
   // sleep_deep();

    
 while(1){

    
    if (BUT_1_pin == 1) {
      x=11;
      send_data(1, &x);
      delay_ms(500);
      x = 1;
     }
    if (BUT_2_pin == 1) {
      x=22;
      send_data(1, &x);
      delay_ms(500);
      x = 1;
     }
    if (BUT_3_pin == 1) {
      x=33;
      send_data(1, &x);
      delay_ms(500);
     }
     if (BUT_4_pin == 1) {
      x=44;
      send_data(1, &x);
      delay_ms(500);
     }
    if (BUT_5_pin == 1) {
      x=55;
      send_data(1, &x);
      delay_ms(500);
     }
     if (BUT_6_pin == 1) {
      x=66;
      send_data(1, &x);
      delay_ms(500);
     }
     
     if (BUT_7_pin == 1) {
      x=77;
      send_data(1, &x);
      delay_ms(500);
     }
     if (BUT_8_pin == 1) {
      x=88;
      send_data(1, &x);
      delay_ms(500);
     }
     if (BUT_9_pin == 1) {
      x=99;
      send_data(1, &x);
      delay_ms(500);
     }
  }
 }
}
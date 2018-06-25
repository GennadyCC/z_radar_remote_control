

unsigned short temp;
unsigned short temp_speed_current;
unsigned short temp_speed_target;



//sbit RF_IRQ_TRIS at TRISB2_bit; //RB0, RB1  SPI
sbit RF_CS_TRIS at TRISC2_bit;
sbit RF_CE_TRIS at TRISC1_bit;     //2
sbit SCK_TRIS at TRISC3_bit;
sbit SDI_TRIS at TRISC4_bit;       //  c7
sbit SDO_TRIS at TRISC5_bit;

sbit LED_TRIS at TRISA5_bit;
sbit IR_out_TRIS at TRISA4_bit;
sbit AUDIO_TRIS at TRISC0_bit;

//sbit IRQ_pin at RB2_bit; //RB0, RB1 SPI
sbit SS_pin at RC2_bit;
sbit CE_pin at RC1_bit;       //2
sbit SCK_pin at RC3_bit;
sbit MOSI_pin at RC4_bit; //SDO  c7
sbit MISO_pin at RC5_bit; //SDI 

sbit LED_pin at RA5_bit;
sbit IR_out at RA4_bit;
sbit AUDIO_pin at RC0_bit;


//sbit Led at RC1_bit;

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
unsigned char x_temp = 0;
unsigned long  count_sleep_pause = 0;
unsigned long  count_led_pulse = 0;
unsigned char s = 0;


unsigned char nRF24L01_read()
{
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


void nRF24L01_write(unsigned char d)
{
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


void register_write(unsigned char reg, unsigned char value)
{
  SS_pin = 0;
  nRF24L01_write((reg | W_REGISTER_cmd));
  nRF24L01_write(value);
  SS_pin = 1;
  delay_us(8);
}


void write_command(unsigned char cmd)
{
  SS_pin = 0;
  nRF24L01_write(cmd);
  SS_pin = 1;
  delay_us(8);
}


unsigned char register_read(unsigned char reg)
{
  unsigned char value = 0;

  SS_pin = 0;
  nRF24L01_write((reg | R_REGISTER_cmd));
  value = nRF24L01_read();
  SS_pin = 1;
  delay_us(8);

  return value;
}


void set_TX_RX_address(unsigned char *addr, unsigned char bytes, unsigned char reg)
{
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


void flush_TX_RX()
{
  register_write(STATUS_reg, 0x70);
  write_command(FLUSH_TX_cmd);
  write_command(FLUSH_RX_cmd);
}


void send_data(unsigned char bytes, unsigned char *value)
{
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

void receive_data(unsigned char bytes, unsigned char *value)
{
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


void nrF24L01_init_TX()
{
  unsigned char address[5] = {0x99, 0x99, 0x99, 0x99, 0x99};

  CE_pin = 0;

  register_write(SETUP_RETR_reg, 0x00);
  register_write(SETUP_AW_reg, 0x03);
  register_write(RF_SETUP_reg, 0x0E);    //0E-2Mbs 26-250Kbs  06-1Mbs
  register_write(RF_CH_reg, 25);
  register_write(EN_AA_reg, 0x00);
  register_write(CONFIG_reg, 0x38);
  set_TX_RX_address(address, 5, TX_ADDR_reg);
  set_TX_RX_address(address, 5, RX_ADDR_P0_reg);
  flush_TX_RX();

  CE_pin = 1;
}

void nrF24L01_init_RX()
{
  unsigned char address[5] = {0x99, 0x99, 0x99, 0x99, 0x99};

  CE_pin = 0;

  register_write(CONFIG_reg, 0x38);
  register_write(SETUP_RETR_reg, 0x00);
  register_write(SETUP_AW_reg, 0x03);
  register_write(RF_SETUP_reg, 0x0E);    //0E-2Mbs 26-250Kbs  06-1Mbs
  register_write(RF_CH_reg, 25);
  register_write(EN_AA_reg, 0x00);
  register_write(RX_PW_P0_reg, 0x01);
  register_write(CONFIG_reg, 0x3B);
  set_TX_RX_address(address, 5, TX_ADDR_reg);
  set_TX_RX_address(address, 5, RX_ADDR_P0_reg);
  flush_TX_RX();

  CE_pin = 1;
}

unsigned char get_Status_Reg()
{
  return register_read(STATUS_reg);
}

unsigned char val = 0;


 void recive_confirmation(unsigned char code_) {
  unsigned char y = 0;
  nrF24L01_init_TX();
  delay_ms(50);
  x = code_;
  while (y < 10) {
   send_data(1, &x);
   delay_ms(10);
   y++;
  }

  nrF24L01_init_RX();
  delay_ms(500);
}








 void Command_Over_Speed () {   // команда Over Speed
        for(s = 0; s < 9; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 8; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 5; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
 }

 void Command_Left () {   // команда стрелка влево
      for(s = 0; s < 6; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 2; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 2; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
       for(s = 0; s < 4; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
       for(s = 0; s < 2; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 2; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 4; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
 }

 void Command_Right () {   // команда стрелка вправо
      for(s = 0; s < 6; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 3; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
       for(s = 0; s < 4; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
       for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 3; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 4; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
 }

void Command_Mute () {   // команда Mute
      for(s = 0; s < 6; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 2; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
       for(s = 0; s < 5; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
       for(s = 0; s < 2; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 5; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
 }


void Command_City () {   // команда City
      for(s = 0; s < 7; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
       for(s = 0; s < 6; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
       for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 1; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 5; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
 }


 void Command_User_POI () {   // команда User POI
        for(s = 0; s < 10; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
      for(s = 0; s < 8; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(550);
     }
      for(s = 0; s < 4; s++) {
        IR_out = 0;
        delay_us(550);
        IR_out = 1;
        delay_us(1660);
     }
 }
 



 void IR_command(char cmd){
 
  IR_out = 0;
  delay_ms(9);
  IR_out = 1;
  delay_us(4490);
  for(s = 0; s < 10; s++) {
    IR_out = 0;
    delay_us(550);
    IR_out = 1;
    delay_us(550);
  }
  
  if (cmd == 1) {
     Command_Over_Speed (); // команда Over Speed
   }
   
   if (cmd == 2) {
     Command_Left ();    // команда стрелка влево
   }
   
   if (cmd == 3) {
     Command_Right ();    // команда стрелка вправо
   }
   
   if (cmd == 4) {
     Command_Mute ();    // команда Mute
   }
   
   if (cmd == 5) {
     Command_City ();    // команда City
   }

   if (cmd == 6) {
     Command_User_POI ();    // команда User POI
   }

  
    IR_out = 0;
    delay_us(550);
    IR_out = 1;
    delay_ms(43);
    IR_out = 0;
    delay_ms(9);
    IR_out = 1;
    delay_us(2100);
    IR_out = 0;
    delay_us(550);
    IR_out = 1;
 }
 
 void set_speed(unsigned short speed) {
    AUDIO_pin = 1;
    IR_command(1);
    temp_speed_target = speed;

    if (temp_speed_current > temp_speed_target) {
      while(temp_speed_current != temp_speed_target) {
        delay_ms(400);
        IR_command(2);
        temp_speed_current = temp_speed_current - 5;
      }
      EEPROM_Write(0x02, temp_speed_current);
    }
    if (temp_speed_current < temp_speed_target) {
      while(temp_speed_current != temp_speed_target) {
        delay_ms(400);
        IR_command(3);
        temp_speed_current = temp_speed_current + 5;
      }
      EEPROM_Write(0x02, temp_speed_current);
    }
    delay_ms(400);
    IR_command(1);
    AUDIO_pin = 0;
 }





void main(){
 delay_ms(10000);
 
 CMCON0 = 0x07;     // Отключить компараторы

  
 RF_CS_TRIS = 0;
 RF_CE_TRIS = 0;
 SCK_TRIS = 0;
 SDI_TRIS = 0;  //MISO
 SDO_TRIS = 1;  //MOSI

 CE_pin = 0;
 SS_pin = 0;
 SCK_pin = 0;
 MOSI_pin = 0;
 

 nrF24L01_init_RX();
 //nrF24L01_init_TX();

 LED_TRIS = 0; // настроить как выход
 IR_out_TRIS = 0;
 IR_out = 1;
 
 AUDIO_TRIS = 0;
 AUDIO_pin = 0;
 
 x = 1;
 
 start_1:
 delay_ms(3000);
 temp_speed_current = EEPROM_Read(0x02);
 temp_speed_target = 60;

 //AUDIO_pin = 1;
 set_speed(60);
 

  while(1){

  if(get_Status_Reg() == 0x40) {

     receive_data(1, &x);
     LED_pin = 1;
     // команды кнопок пульта
     // 11 - кнопка "пауза, play"
     // 22 - стрелка вниз
     // 33 - кнопка "+"
     // 44 - кнопка SET
     // 55 - кнопка "-"
     // 66 - стрелка влево
     // 77 - кнопка "M"
     // 88 - стрелка вправо
     // 99 - стрелка вверх
     
     // команды на выполнение действий
     // 1 - OVER SPEED
     // 2 - Left
     // 3 - Right
     // 4 - Mute
     // 5 - City
     // 6 - User POI
     // 7 -
     // 8 -
     // 9 -
     
      if(x==77){   // установить скорость 60
         set_speed(60);
       }
      if(x==66){   // установить скорость 40
            set_speed(40);
        }
      if(x==88){    // установить скорость 90
            set_speed(90);
        }
      if(x==99){    // установить скорость 120
            set_speed(120);
        }
      if(x==22){    // установить скорость 100
            set_speed(100);
        }
      
      if(x==33){    // установить скорость +5
            set_speed(temp_speed_current+5);
        }
      if(x==55){    // установить скорость -5
            set_speed(temp_speed_current-5);
        }

      if(x==11){    // Mute
            IR_command(4);
        }
      if(x==44){
          EEPROM_Write(0x02, 60);
          delay_ms(300);
          IR_command(1);
          delay_ms(300);
          IR_command(1);
        }
        
        
       delay_ms(20);
   }

 }

}
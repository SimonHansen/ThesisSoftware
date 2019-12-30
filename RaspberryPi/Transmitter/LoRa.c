/*******************************************************************************
 *
 * This code has been heavily inspired by the rpi-lora-transceiver from dragino
 *
 * https://github.com/dragino/rpi-lora-tranceiver
 *
 *******************************************************************************/

#include "LoRa.h"
#include "GPS.h"

static const int CHANNEL = 0;
char message[256];
byte receivedbytes;

time_t rawtime;

FILE *CSVfile;
char *filename;

uint32_t freq_;
int power_;
sf_t sf_;
uint8_t bw_;
uint8_t cr_;

int ssPin = 6;
int dio0  = 7;
int RST   = 0;

void setParameters(uint32_t freq, int power, sf_t sf, uint8_t bw, uint8_t cr)
{
	freq_ = freq;
	power_ = power;
	sf_ = sf;
	bw_ = bw;
	cr_ = cr;
	config();
}


void die(const char *s)
{
    perror(s);
    exit(1);
}

void selectreceiver()
{
    digitalWrite(ssPin, LOW);
}

void unselectreceiver()
{
    digitalWrite(ssPin, HIGH);
}

byte readReg(byte addr)
{
    unsigned char spibuf[2];

    selectreceiver();
    spibuf[0] = addr & 0x7F;
    spibuf[1] = 0x00;
    wiringPiSPIDataRW(CHANNEL, spibuf, 2);
    unselectreceiver();

    return spibuf[1];
}

void writeReg(byte addr, byte value)
{
    unsigned char spibuf[2];

    spibuf[0] = addr | 0x80;
    spibuf[1] = value;
    selectreceiver();
    wiringPiSPIDataRW(CHANNEL, spibuf, 2);

    unselectreceiver();
}

void opmode(uint8_t mode) {
	if(mode != OPMODE_LORA)
    writeReg(REG_OPMODE, (readReg(REG_OPMODE) & ~OPMODE_MASK) | mode);
    else
    writeReg(REG_OPMODE, mode);
}

static void configPower (int8_t pw) {
        // power needs to be between 17 and 2
        if(pw >= 17) {
            pw = 15;
        } else if(pw < 2) {
            pw = 2;
        }
        // check board type for BOOST pin
        writeReg(RegPaConfig, (uint8_t)(0x80|(pw&0xf)));
        writeReg(RegPaDac, readReg(RegPaDac)|0x4);

}

void config()
{
	// Setup WiringPi interface to LoRa HAT
	wiringPiSetup () ;
    pinMode(ssPin, OUTPUT);
    pinMode(dio0, INPUT);
    pinMode(RST, OUTPUT);
    wiringPiSPISetup(CHANNEL, 500000);
    
	digitalWrite(RST, LOW);
	delay(100);
	digitalWrite(RST, HIGH);
	delay(100);
	// Check if version number is correct
	byte version = readReg(REG_VERSION);
	if (version == 0x12) {
		// sx1276
		printf("SX1276 detected, starting.\n");
	} else {
		printf("Unrecognized transceiver.\n");
		exit(1);
	}
	
	// Set the mode to LoRa 
    opmode(OPMODE_LORA);

    // set frequency
    uint64_t frf = ((uint64_t)freq_ << 19) / 32000000;
    writeReg(REG_FRF_MSB, (uint8_t)(frf>>16) );
    writeReg(REG_FRF_MID, (uint8_t)(frf>> 8) );
    writeReg(REG_FRF_LSB, (uint8_t)(frf>> 0) );
  
	// set the spreading factor if SF11 or SF12
	if (sf_ == SF11 || sf_ == SF12) {
		writeReg(REG_MODEM_CONFIG3,0x0C);
	} else {
		writeReg(REG_MODEM_CONFIG3,0x04);
	}
	// Set bandwidth and codingrate
	writeReg(REG_MODEM_CONFIG, (bw_<<4) | (cr_ <<1) | 0x00);
	// set the spreading factor
	writeReg(REG_MODEM_CONFIG2,(sf_<<4) | 0x04);

	// Set the RX timeout 
    if (sf_ == SF10 || sf_ == SF11 || sf_ == SF12) {
        writeReg(REG_SYMB_TIMEOUT_LSB,0x05);
    } else {
        writeReg(REG_SYMB_TIMEOUT_LSB,0x08);
    }
	// Set the max payload length
    writeReg(REG_MAX_PAYLOAD_LENGTH,0xFF); 
	// Payload length
    writeReg(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);
	// Period between hops
    writeReg(REG_HOP_PERIOD,0xFF);
	// LNA gain
    writeReg(REG_FIFO_ADDR_PTR, readReg(REG_FIFO_RX_BASE_AD));
    writeReg(REG_LNA, LNA_MAX_GAIN);
	 // Config Power level
    configPower(power_);
    
    
    
}


bool receive(char *payload) {
    // clear rxDone
    writeReg(REG_IRQ_FLAGS, 0x40);

    int irqflags = readReg(REG_IRQ_FLAGS);

    //  payload crc: 0x20
    if((irqflags & 0x20) == 0x20)
    {
        printf("CRC error\n");
        writeReg(REG_IRQ_FLAGS, 0x20);
        return false;
    } else {

        byte currentAddr = readReg(REG_FIFO_RX_CURRENT_ADDR);
        byte receivedCount = readReg(REG_RX_NB_BYTES);
        receivedbytes = receivedCount;

        writeReg(REG_FIFO_ADDR_PTR, currentAddr);

        for(int i = 0; i < receivedCount; i++)
        {
            payload[i] = (char)readReg(REG_FIFO);
        }
    }
    return true;
}

void receivepacket() {

	
    long int SNR;
    int rssicorr;

    if(digitalRead(dio0) == 1)
    {
        if(receive(message)) {
            byte value = readReg(REG_PKT_SNR_VALUE);
            if( value & 0x80 ) // The SNR sign bit is 1
            {
                // Invert and divide by 4
                value = ( ( ~value + 1 ) & 0xFF ) >> 2;
                SNR = -value;
            }
            else
            {
                // Divide by 4
                SNR = ( value & 0xFF ) >> 2;
            }
            
            rssicorr = 157;

			    //int gps = 0;
	    CSVfile = fopen(filename , "a");
	    fprintf(CSVfile, "\n%d; %d; %li; %s; %s; %d; %d; %d; %d",readReg(0x1A)-rssicorr,readReg(0x1B)-rssicorr, SNR, message, gps, freq_, bw_, power_, sf_);
	    fclose(CSVfile);	
			
        } // received a message

    }
}



static void writeBuf(byte addr, byte *value, byte len) {                                                       
    unsigned char spibuf[256];                                                                          
    spibuf[0] = addr | 0x80;                                                                            
    for (int i = 0; i < len; i++) {                                                                         
        spibuf[i + 1] = value[i];                                                                       
    }                                                                                                   
    selectreceiver();                                                                                   
    wiringPiSPIDataRW(CHANNEL, spibuf, len + 1);                                                        
    unselectreceiver();                                                                                 
}

void transmitpacket(byte *frame, byte datalen) {

    // Map DIO to TxDone - IRQ
    writeReg(RegDioMapping1, MAP_DIO0_LORA_TXDONE|MAP_DIO1_LORA_NOP|MAP_DIO2_LORA_NOP);
    // Clear interrupts flags
    writeReg(REG_IRQ_FLAGS, 0xFF);
    // Disable all interrupts expect TxDone
    writeReg(REG_IRQ_FLAGS_MASK, ~IRQ_LORA_TXDONE_MASK);

    // Specific  where transmit information is stored
    writeReg(REG_FIFO_TX_BASE_AD, 0x00);
    writeReg(REG_FIFO_ADDR_PTR, 0x00);
	// Length of data to be sent
    writeReg(REG_PAYLOAD_LENGTH, datalen);
	// Write payload to FIFO
    writeBuf(REG_FIFO, frame, datalen);
    // Set mode to transmit and transmit message
    opmode(OPMODE_TX);

    
}


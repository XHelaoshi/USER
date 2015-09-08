#include "avdtp.h"
#include "a2dp.h"

#include <rtthread.h>
#include "debug.h"
#include "hci.h"
#include "rfcomm.h"



AVDTP_STATE_t machine_state = AVDTP_State_Idle;


int fd_o;
static uint8_t acp_seid=1,int_seid=0,open_dev=0;
static uint8_t a2dp_inited=0;
static uint8_t *sbc_buffer;
void avdtp_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

//extern void sbcc(const char * name);
//extern void sbc_stop();
//extern void sbc_pause();
//extern void sbc_play();
//extern int sbc_fifo_write(uint8_t *dat, int size);

void realease_resource(void){
//	sbc_stop();
	if(sbc_buffer)
	{
		rt_free(sbc_buffer);
		sbc_buffer = NULL;
	}
	machine_state = AVDTP_State_Idle;
	open_dev = 0;
}



static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	
	if(packet_type == AVDTP_SIGNAL_PACKET)
	{
	switch (packet[1]&0x3f){
		case AVDTP_Discover:
			if(a2dp_inited==1)
			{
				uint8_t data[2] = {(acp_seid<<2)|0x00, AVDTP_MEDIA_TYPE_AUDIO<<4 | AVDTP_SEP_TYPE_SINK << 3 };
				avdtp_accept(channel,data,2);
				machine_state = AVDTP_State_Idle;
				log_info("AVDTP_State_Idle. ");
			}
				break;
		
		case AVDTP_Get_Capabilities:
		{
				uint8_t data[10] = {0x01,0x00,0x07,0x06,0x00,0x00,0x3f,0xff,0x02,0x53};
				avdtp_accept(channel,data,10);
			}
			break;
		
		case AVDTP_Set_Configuration:
		{	
			if(machine_state == AVDTP_State_Idle)
			{
				machine_state = AVDTP_State_Configured;
				log_info("AVDTP_State_Configured. \n");
				int_seid = packet[3]>>2;
				avdtp_accept(channel,NULL,0);
			}
			log_info("Get AVDTP Configuration, the INT supports: \n");
			log_info("Sampling Frequency: \n");
			if(packet[10]&0x80) log_info("16000 ");
			if(packet[10]&0x40) log_info("32000 ");
			if(packet[10]&0x20) log_info("44100 ");
			if(packet[10]&0x10) log_info("48000 ");
			log_info("\n");
			
			log_info("Channel Mode: \n");
			if(packet[10]&0x8) log_info("MONO ");
			if(packet[10]&0x4) log_info("DUAL CHANNEL ");
			if(packet[10]&0x2) log_info("STEREO ");
			if(packet[10]&0x1) log_info("JOINT STEREO ");
			log_info("\n");
			
			log_info("Block Length: \n");
			if(packet[11]&0x80) log_info("4 ");
			if(packet[11]&0x40) log_info("8 ");
			if(packet[11]&0x20) log_info("12 ");
			if(packet[11]&0x10) log_info("16 ");
			log_info("\n");			

			log_info("Subbands: \n");
			if(packet[11]&0x8) log_info("4 ");
			if(packet[11]&0x4) log_info("8 ");
			log_info("\n");	
			
			log_info("Allocation Method: \n");
			if(packet[11]&0x2) log_info("SNR ");
			if(packet[11]&0x1) log_info("Loudness ");
			log_info("\n");			
			

			
		}
			break;
		
		case AVDTP_Get_Configuration:
			break;
		
		case AVDTP_Reconfigure:
		{	
			if(machine_state == AVDTP_State_Open)
			{
				machine_state = AVDTP_State_Open;
				log_info("AVDTP_State_Open. \n");
				int_seid = packet[3]>>2;
				avdtp_accept(channel,NULL,0);
			}
			log_info("Get AVDTP Configuration, the INT supports: \n");
			log_info("Sampling Frequency: \n");
			if(packet[10]&0x80) log_info("16000 ");
			if(packet[10]&0x40) log_info("32000 ");
			if(packet[10]&0x20) log_info("44100 ");
			if(packet[10]&0x10) log_info("48000 ");
			log_info("\n");
			
			log_info("Channel Mode: \n");
			if(packet[10]&0x8) log_info("MONO ");
			if(packet[10]&0x4) log_info("DUAL CHANNEL ");
			if(packet[10]&0x2) log_info("STEREO ");
			if(packet[10]&0x1) log_info("JOINT STEREO ");
			log_info("\n");
			
			log_info("Block Length: \n");
			if(packet[11]&0x80) log_info("4 ");
			if(packet[11]&0x40) log_info("8 ");
			if(packet[11]&0x20) log_info("12 ");
			if(packet[11]&0x10) log_info("16 ");
			log_info("\n");			

			log_info("Subbands: \n");
			if(packet[11]&0x8) log_info("4 ");
			if(packet[11]&0x4) log_info("8 ");
			log_info("\n");	
			
			log_info("Allocation Method: \n");
			if(packet[11]&0x2) log_info("SNR ");
			if(packet[11]&0x1) log_info("Loudness ");
			log_info("\n");			
			

			
		}

			break;
		
		case AVDTP_Open:
			if((machine_state == AVDTP_State_Configured) || (machine_state == AVDTP_State_Streaming)){
//				sbc_pause();
//				sbcc("/dev/stream");
				open_dev = 1;
				rt_thread_delay(2);
				avdtp_accept(channel,NULL,0);
				
				machine_state = AVDTP_State_Open;
				
				log_info("AVDTP_State_Open. \n");
			}
			break;
		
		case AVDTP_Start:
			if(machine_state == AVDTP_State_Open){
				avdtp_accept(channel,NULL,0);
//				fd_o = open("/sd/dat.ljj", O_WRONLY|O_CREAT, 0);
//					if(fd_o < 0)
//		log_info("open dat.ljj fail.");
				machine_state = AVDTP_State_Streaming;
				log_info("AVDTP_State_Streaming. \n");
			}
			break;
		
		case AVDTP_Close:
			if(machine_state == AVDTP_State_Open){
				avdtp_accept(channel,NULL,0);
				machine_state = AVDTP_State_Closing;
				log_info("AVDTP_State_Closing. \n");
				realease_resource();
			}
		
			break;
		
		case AVDTP_Suspend:
			if(machine_state == AVDTP_State_Streaming){
//				if(fd_o >= 0) close(fd_o);
				

				if(open_dev==0)
				{
//					sbcc("/dev/stream");
					rt_thread_delay(2);
					open_dev = 1;
				}
				avdtp_accept(channel,NULL,0);
				machine_state = AVDTP_State_Open;
				log_info("AVDTP_State_Open. \n");
			}
			break;
		
		case AVDTP_abort:
				if((machine_state == AVDTP_State_Open) || (machine_state == AVDTP_State_Configured)\
					|| (machine_state == AVDTP_State_Streaming) || (machine_state == AVDTP_State_Closing)){
				avdtp_accept(channel,NULL,0);
				machine_state = AVDTP_State_Aborting;
				log_info("AVDTP_State_Aborting. \n");
				realease_resource();
			}
			break;
		
		case AVDTP_Security_Control:
			break;
		
		default:
			break;
		
		
		
		}
	}
}
int tim=0;
uint8_t *add;
void dreamer_packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
//	write(fd_o,&packet[13],size-13);
	if(sbc_buffer==NULL)
	{
		sbc_buffer = rt_malloc((size-13)*3);
		add = sbc_buffer;
	}
	
	switch(tim%3){
		case 0:
			rt_memcpy(sbc_buffer,&packet[13],size-13);
			sbc_buffer += size-13;
		break;
		case 1:
			rt_memcpy(sbc_buffer,&packet[13],size-13);
			sbc_buffer += size-13;
			break;
		case 2:
			rt_memcpy(sbc_buffer,&packet[13],size-13);
			sbc_buffer += size-13;
//			sbc_fifo_write(add,sbc_buffer-add);
			sbc_buffer = add;
			break;
		default:
			break;
	}
	tim++;
	if(tim==3);
//	sbc_play();
	
//	if(fd_o >= 0) close(fd_o);
}

void a2dp_init(){
	
	avdtp_register_packet_handler(avdtp_packet_handler);
	dreamer_register_packet_handler(dreamer_packet_handler);


	avdtp_init(acp_seid);
	a2dp_inited = 1;

}

void avdtp_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    packet_handler(packet_type, channel, packet, size);
}




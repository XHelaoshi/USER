
#include <rtthread.h>
//#include "global.h"
#include "debug.h"
#include "hci.h"
#include "rfcomm.h"
#include "spp.h"
#include "stdint.h"
#include "stdio.h"
#include "finsh.h"
#include "string.h"
#include "BTStack\Image.h"
#include "hci_transport_h4_dma.h"
static bd_addr_t remote = {0x84, 0x38, 0x35, 0x65, 0xD1, 0x15};
static uint8_t test[8]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
static uint8_t rfcomm_channel_nr = 1;

static volatile uint16_t  rfcomm_channel_id;

static uint16_t  mtu;
//static char *AT_cmd;
static uint16_t rfcomm_spp_cid = 0;
rt_bool_t client_ready = RT_FALSE;
extern int h4_can_send_packet_now(uint8_t packet_type);
 rt_tick_t tt=0; //use for debug
extern int rfcommcreditsready;
/*deal with spp rfcomm data*/
static void spp_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	    uint16_t psm;
    uint32_t passkey;
		uint8_t test_data[16] ={0x41, 0x54, 0x2B, 0x43, 0x4D, 0x45, 0x52, 0x3D, 0x33, 0x2C, 0x30, 0x2C, 0x30, 0x2C, 0x31, 0x0d};
//    if (packet_type != HCI_EVENT_PACKET) return;
		rfcomm_spp_cid = channel;
		rfcomm_channel_t * channel1 = rfcomm_channel_for_rfcomm_cid(channel);
		rt_kprintf("SPP Channel id is:%d dlci is:%d\n\r",channel,channel1->dlci);
		if(packet_type == HCI_EVENT_PACKET)
		{
			
			switch (packet[0]) {
					case RFCOMM_EVENT_INCOMING_CONNECTION:
							// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
							bt_flip_addr(remote, &packet[2]); 
							rfcomm_channel_nr = packet[8];
							rfcomm_channel_id = READ_BT_16(packet, 9);
							log_info("spp_RFCOMM channel %u requested for %s\n\r", rfcomm_channel_nr, bd_addr_to_str(remote));
							rfcomm_accept_connection_internal(rfcomm_channel_id);
							break;
							
					case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
							// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
							if (packet[2]) {
									log_info("RFCOMM channel open failed, status %u\n\r", packet[2]);
							} else {
									rfcomm_channel_id = READ_BT_16(packet, 12);
									mtu = READ_BT_16(packet, 14);
									if (mtu > 120){
											log_info("BTstack libusb hack: using reduced MTU for sending instead of %u\n", mtu);
											mtu = 120;
									}
									log_info("\n\rRFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n\r", rfcomm_channel_id, mtu);
							}
							break;
							
					case RFCOMM_EVENT_PORT_CONFIGURATION:
						{
							rfcomm_rpn_data_t *rpn_data = (rfcomm_rpn_data_t *)&packet[2];
							uint16_t rfcomm_cid = channel;
							rpn_data_bits_t data_bits = rpn_data->flags & 0x03;
							rpn_stop_bits_t stop_bits = (rpn_data->flags >> 2) & 0x01;
							rpn_parity_t parity = (rpn_data->flags >> 3) & 0x07;
							log_info("spp_RFCOMM channel configuration send.");
							rfcomm_send_port_configuration(rfcomm_cid, rpn_data->baud_rate, data_bits, stop_bits, parity,  rpn_data->flow_control);
						}
							break;
							
					case RFCOMM_EVENT_SEND_HSP_INIT:
								
//								if(AT_cmd==NULL)
//								{
//									AT_cmd = rt_malloc(sizeof(char)*16);
//								}
//								rt_memset(AT_cmd,0,16);
//								rt_memcpy(AT_cmd,"AT+CMER=3,0,0,1",16);
////								rt_memcpy(AT_cmd,"AT+BRSF=0",10);
////									rt_memcpy(AT_cmd,"AT+CIND=?",10);

//							  AT(AT_cmd);
					
//								rfcomm_send_internal(channel,test_data,16);
						break;
					case RFCOMM_EVENT_CHANNEL_CLOSED:
							rfcomm_channel_id = 0;
							break;

					default:
							break;
			}
		}
		else if(packet_type == RFCOMM_DATA_PACKET)
		{
			int i;
			rt_kprintf("spp RFCOMM_DATA_PACKET %c ",channel);
			for(i = 0; i < size; i++)
			{
				rt_kprintf("%c",packet[i]);
			}
			rt_kprintf("\n\r");
			spp_data_handler(packet, size);
		}
}
void spp_rfcomm_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    spp_packet_handler(packet_type, channel, packet, size);
	//rt_kprintf("connection:%c\n\r",&connection);
}

/*spp data process*/
void spp_data_handler(uint8_t *packet, uint16_t size){
		switch(packet[0]){
			case SPP_CLIENT_OK:
				client_ready = RT_TRUE;
				rt_kprintf("Client ok!\n\r");
				break;
			case SPP_CLIENT_NO:
				client_ready = RT_FALSE;
				rt_kprintf("Client wrong!\n\r");
				break;
			case SPP_OP:
				spp_op_data_handler(&packet[1],size-1);
			default:break;
		}
}
/*
	MCU receive opcode process
	Only print now!
*/
void spp_op_data_handler(uint8_t *packet, uint16_t size)
{
	int len;
	int i;
	for(i = 0; i <= size; i++)
		rt_kprintf("%c",packet[i]);
}
/*send message data*/
void SM(uint8_t *str)
{
	int i;
	int len;
	len = rt_strlen(str);
	str[len] = 0x0D;
	rfcomm_send_internal(rfcomm_channel_id,str,len);
	 for(i = 0; i <= len; i++)
    rt_kprintf("%c",str[i]);
}
/***************************************
* @function add header of data to send
* @param str          data to be send    
* @param header				1 bit header to add
* @param dst          complete data   
* @param size					unadded data size				
****************************************/
void spp_header_prepare(uint8_t * str, uint8_t header,uint8_t * dst,uint16_t size)
{
	int i;
	dst[0]=header;
	for(i=0;i<size;i++){
		dst[i+1] = str[i];
	}
}
int SM_Head(uint8_t *str, uint8_t header,uint32_t len)
{
	if(str == NULL){
		uint8_t temp[1];
		temp[0] = header;
		//rfcomm_send_internal(rfcomm_channel_id,temp,1);
			if(rfcomm_send_internal(rfcomm_channel_id,temp,1)!=0)
		return 1;
	    else 
		return 0;
		rt_kprintf("Only send head:%02x\n\r",header);
	}
	int i;
	//len = rt_strlen(str);
	uint8_t temp[len+1];
	spp_header_prepare(str,header,temp,len);
	rt_kprintf("Pure Data length : %d Send data is:",len);
	for(i=0;i<len+1;i++)
	{
		rt_kprintf("%x ",temp[i]);
	}
	rt_kprintf("\n\r");
	//rfcomm_send_internal(rfcomm_channel_id,temp,len);
		if(rfcomm_send_internal(rfcomm_channel_id,temp,len+1)!=0)
		return 1;
	else 
		return 0;
}

/*send message data of length*/
int SM_Length(uint8_t *str,uint16_t size, uint8_t header)
{
	if(size == 0)return 1;
	int i;
	uint8_t temp[size+1];
	spp_header_prepare(str,header,temp,size);
//	rt_kprintf("Send data is:");
//	for(i=0;i<size+1;i++)
//	{
//		rt_kprintf("%c",temp[i]);
//	}
	rt_kprintf("\n\r");
	if(rfcomm_send_internal(rfcomm_channel_id,temp,size+1)!=0)
		return 1;
	else 
		return 0;
}

void int2str(int i, char *s) {
  sprintf(s,"%d",i);
}
/***************************************
* @function send data 
* @param data user data
* @param size	user data size without header
* @param mtu_t	max size of one package without header
****************************************/
void spp_send_data(uint8_t *data, uint32_t size, uint16_t mtu_t)
{
	uint8_t packed_type;
	uint8_t tempsize[6];
	int len_t, len_last;
	int i,j;
	tt=0;
	len_t = size / mtu_t;
	len_last = size - len_t*mtu_t;//last package length
	len_t ++;//caculate how many packages should be sent;
	tempsize[0] = (size>>24)&0xff;
	tempsize[1] = (size>>16)&0xff;
	tempsize[2] = (size>>8)&0xff;
	tempsize[3] = (size)&0xff;
	
	tempsize[4] = (len_t>>8)&0xff;
	tempsize[5] = (len_t)&0xff;	
	while(!can_send_packet_now());
	SM_Head(tempsize,SPP_DATA_LENGTH,6);//send data length with SPP_DATA_LENGTH head
	while(!client_ready){};	
	packed_type = 0x02;//pure data header 
	//only 1 package to be sent
	if(size <= mtu_t){
		rt_kprintf("Sending Data Package 1/1..\n\r");
		while(!can_send_packet_now());
		SM_Head(data,packed_type,size);
		rt_kprintf("Sending Data Package 1/1...OK\n\r");
		return;
	}
	// more than 1 package
	else{
		rt_tick_t t1 = rt_tick_get();
//		int len_t, len_last;
//		int i,j;
//		tt=0;
//		len_t = size / mtu_t;
//		len_last = size - len_t*mtu_t;//last package length
//		len_t ++;//caculate how many packages should be sent;
		for(i=0,j=0; i<len_t; i++,j++){
			// wait until client message received ok	  
				//while(!client_ready){};	
			  while(rfcommcreditsready){}
				while(!can_send_packet_now()){}
				rfcommcreditsready=1;
				rt_kprintf("Client OK\n\r");
				client_ready = RT_FALSE;
				//SM(&packed_type);
				rt_kprintf("Sending Data Package %d/%d..\n\r", i+1, len_t );
				if(i == len_t-1){
					//SM_Length(&data[i*mtu_t], len_last,packed_type);		
					rfcomm_send_internal_head_index(rfcomm_channel_id, &data[i*mtu_t],len_last+3 ,packed_type,i);
				}
				else{
					//SM_Length(&data[i*mtu_t], mtu_t, packed_type);	
					rfcomm_send_internal_head_index(rfcomm_channel_id, &data[i*mtu_t],mtu_t+3,packed_type,i);					
				}
				//rt_kprintf("Wait for client...\n\r");
			
		}
		rt_kprintf("Sending Data Complite!");
		rt_tick_t t2 = rt_tick_get();
	  tt = t2-t1;
		
	}
	client_ready = RT_FALSE;
	while(!client_ready){};
	while(!can_send_packet_now());
	packed_type = SPP_SEND_DONE;
	SM_Head(NULL,packed_type,0);
}
/***************************************
* @function send data origion (with ack of phone.slow)
* @param data user data
* @param size	user data size without header
* @param mtu_t	max size of one package without header
****************************************/
/*void spp_send_data(uint8_t *data, uint16_t size, uint16_t mtu_t)
{
	uint8_t packed_type;
	uint8_t tempsize[2];
	tempsize[0] = (size>>8)&0xff;
	tempsize[1] = (size)&0xff;
	//tempsize[2] = 0xff;
	while(!can_send_packet_now());
	SM_Head(tempsize,SPP_DATA_LENGTH);//send data length with SPP_DATA_LENGTH head
	while(!client_ready){};	
	packed_type = 0x02;//pure data header 
	//only 1 package to be sent
	if(size <= mtu_t){
		rt_kprintf("Sending Data Package 1/1..\n\r");
		while(!can_send_packet_now());
		SM_Head(data,packed_type);
		rt_kprintf("Sending Data Package 1/1...OK\n\r");
		return;
	}
	// more than 1 package
	else{
		rt_tick_t t1 = rt_tick_get();
		int len_t, len_last;
		int i,j;
		tt=0;
		len_t = size / mtu_t;
		len_last = size - len_t*mtu_t;//last package length
		len_t ++;//caculate how many packages should be sent;
		for(i=0,j=0; i<len_t; i++,j++){
			// wait until client message received ok	  
				//while(!client_ready){};				
			  while(rfcommcreditsready){}
				while(!can_send_packet_now()){}
				rfcommcreditsready=1;
				rt_kprintf("Client OK\n\r");
				client_ready = RT_FALSE;
				//SM(&packed_type);
				rt_kprintf("Sending Data Package %d/%d..\n\r", i+1, len_t );
				if(i == len_t-1){
					//SM_Length(&data[i*mtu_t], len_last,packed_type);		
					rfcomm_send_internal_head(rfcomm_channel_id, &data[i*mtu_t],len_last+1 ,packed_type);
				}
				else{
					//SM_Length(&data[i*mtu_t], mtu_t, packed_type);	
					rfcomm_send_internal_head(rfcomm_channel_id, &data[i*mtu_t],mtu_t+1,packed_type);					
				}
				rt_kprintf("Wait for client...\n\r");
			
		}
		rt_kprintf("Sending Data Complite!");
		rt_tick_t t2 = rt_tick_get();
	  tt = t2-t1;
		
	}
	client_ready = RT_FALSE;
	while(!client_ready){};
	while(!can_send_packet_now());
	packed_type = SPP_SEND_DONE;
	SM_Head(NULL,packed_type);
}*/

/***************************************
* @function 	send file's name with header of SPP_DATA_NAME
* @param		name 		file's name
****************************************/
void spp_send_name(uint8_t* name,uint16_t len)
{
	if(name == NULL)return;
	SM_Head(name,SPP_DATA_NAME,len);
	rt_kprintf("Data Name send OK!\n\r");
}

void gettick()
{
	rt_kprintf("Use total of rt_time tick:%d\n\r",tt);
}

void getmtu()
{
	rt_kprintf("%d\n\r",mtu);
}

void st()
{
	spp_send_data((uint8_t*)gImage_testimage,8074,mtu-3);//3252
}	
void st1()
{
	spp_send_data((uint8_t*)gImage_testimage1,42390,mtu-3);//3252
}
void sn(uint8_t *str)
{
	int len=rt_strlen(str);
	if(len>mtu){
		rt_kprintf("Name too long!ERROR\n\r");
		return;
	}
	spp_send_name(str,len);
}
void getid()
{
	rt_kprintf("channel id:%d\n\r",rfcomm_channel_id);
}
FINSH_FUNCTION_EXPORT(getid,getid)
FINSH_FUNCTION_EXPORT(SM, SendMessage)
FINSH_FUNCTION_EXPORT(getmtu, getmtu)
FINSH_FUNCTION_EXPORT(st, send data)
FINSH_FUNCTION_EXPORT(st1, send data don not stop)
FINSH_FUNCTION_EXPORT(sn, send data name)
FINSH_FUNCTION_EXPORT(gettick, get total of tick)
//void AT(char * str)
//{
//  int i;
//  int len;
//  //char s[32];
//  len = rt_strlen(str);
//  str[len] = 0x0D;
//  RFCOMM_API_sendData(str, len + 1);
//  for(i = 0; i <= len; i++)
//    rt_kprintf("%c",str[i]);
//}

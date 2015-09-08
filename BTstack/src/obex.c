
#include <rtthread.h>
//#include "global.h"
#include "debug.h"
#include "hci.h"
#include "rfcomm.h"
#include "obex.h"
#include "stdint.h"

static bd_addr_t remote = {0x84, 0x38, 0x35, 0x65, 0xD1, 0x15};
static uint8_t rfcomm_channel_nr = 1;
static uint16_t  rfcomm_channel_id;
static uint16_t  mtu;
//static char *AT_cmd;
static uint16_t rfcomm_obex_cid = 0;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	    uint16_t psm;
    uint32_t passkey;
		uint8_t test_data[16] ={0x41, 0x54, 0x2B, 0x43, 0x4D, 0x45, 0x52, 0x3D, 0x33, 0x2C, 0x30, 0x2C, 0x30, 0x2C, 0x31, 0x0d};
//    if (packet_type != HCI_EVENT_PACKET) return;
		rfcomm_obex_cid = channel;
		if(packet_type == HCI_EVENT_PACKET)
		{
			
			switch (packet[0]) {
					case RFCOMM_EVENT_INCOMING_CONNECTION:
							// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
							bt_flip_addr(remote, &packet[2]); 
							rfcomm_channel_nr = packet[8];
							rfcomm_channel_id = READ_BT_16(packet, 9);
							log_info("Obex_RFCOMM channel %u requested for %s\n\r", rfcomm_channel_nr, bd_addr_to_str(remote));
							rfcomm_accept_connection_internal(rfcomm_channel_id);
							break;
							
					case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
							// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
							if (packet[2]) {
									log_info("RFCOMM channel open failed, status %u\n\r", packet[2]);
							} else {
									rfcomm_channel_id = READ_BT_16(packet, 12);
									mtu = READ_BT_16(packet, 14);
									if (mtu > 60){
											log_info("BTstack libusb hack: using reduced MTU for sending instead of %u\n", mtu);
											mtu = 60;
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

							
							log_info("obex_RFCOMM channel configuration send.");
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

		}
}

void obex_rfcomm_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    packet_handler(packet_type, channel, packet, size);
}
void spp_rfcomm_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    packet_handler(packet_type, channel, packet, size);
}
#include "finsh.h"
#include "string.h"
void SM(uint8_t *str)//sendmessage
{
	int i;
	int len;
	len = rt_strlen(str);
	str[len] = 0x0D;//»Ø³µ
	rfcomm_send_internal(rfcomm_channel_id,str,len);
	 for(i = 0; i <= len; i++)
    rt_kprintf("%c",str[i]);
}
FINSH_FUNCTION_EXPORT(SM, SendMessage)
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
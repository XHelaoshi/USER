#include "avctp.h"
#include "btstack_memory.h"
#include <btstack/hci_cmds.h>

#include <stdio.h>
#include <string.h>
#include "hci_dump.h"
#include "l2cap.h"

#include "debug.h"

#define AVCTP_RESPONSE_BUFFER_SIZE (HCI_ACL_BUFFER_SIZE-HCI_ACL_HEADER_SIZE)

static void avctp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static void (*app_packet_handler)(void * connection, uint8_t packet_type,
                                  uint16_t channel, uint8_t *packet, uint16_t size);
static void avctp_try_respond();




static uint16_t l2cap_cid=0;
static uint16_t avctp_response_size = 0;
static uint8_t avctp_response_buffer[AVCTP_RESPONSE_BUFFER_SIZE];

void avctp_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type,
                                                    uint16_t channel, uint8_t *packet, uint16_t size)){
	app_packet_handler = handler;
}
																										
void avctp_init(void){
    // register with l2cap psm sevices - max MTU
    l2cap_register_service_internal(NULL, avctp_packet_handler, PSM_AVCTP, 0x30, LEVEL_0);
//		signaling_responses_pending = 0;
//	avdtp_channl.state = AVDTP_State_Idle;
//	avdtp_channl.avdtp_signaling.acp_seid = acp_seid;
	
}

//void avctp_accept(uint16_t avdtp_cid, uint8_t *data, uint16_t len){

//		avdtp_response_buffer[0] = (avdtp_signaling_respons.transaction_label << 4) |\
//													AVDTP_PKT_TYPE_SINGLE | AVDTP_MESSAGE_ACCEPT;
//		avdtp_response_buffer[1] = avdtp_signaling_respons.signal_identifier;
//		for(uint8_t i=0;i<len;i++)
//		{
//			avdtp_response_buffer[2+i] = data[i];	
//		}
//		chan->avdtp_response_size = len + 2;
//}


static void avctp_hci_event_handler(uint16_t channel,uint8_t *packet, uint16_t size){
	
			switch (packet[0]) {
				case L2CAP_EVENT_INCOMING_CONNECTION:
				{
					if (l2cap_cid) {
						// CONNECTION REJECTED DUE TO LIMITED RESOURCES 
						l2cap_decline_connection_internal(channel, 0x04);
						break;
				}
					l2cap_cid = channel;
					l2cap_accept_connection_internal(channel);
					avctp_response_size = 0;
					break;
				}
				case L2CAP_EVENT_CHANNEL_OPENED:
						if (packet[2]) {
								// open failed -> reset
								l2cap_cid = 0;
						}
						break;

                case L2CAP_EVENT_CREDITS:
                case DAEMON_EVENT_HCI_PACKET_SENT:
                    avctp_try_respond();
                    break;
//                
				case L2CAP_EVENT_CHANNEL_CLOSED:
				{
						if (channel == l2cap_cid){
								// reset
								l2cap_cid = 0;
						}
				}
						break;
					                    
				default:
					// other event
					break;
			}
}

static void avctp_l2cap_packet_handler(uint16_t channel, uint8_t *packet, uint16_t size){

		(*app_packet_handler)(NULL,AVCTP_SIGNAL_PACKET, channel, packet, size);

}


// we assume that we don't get two requests in a row
static void avctp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){

    switch (packet_type) {
        case HCI_EVENT_PACKET:
							avctp_hci_event_handler(channel, packet, size);
							
            break;
        case L2CAP_DATA_PACKET:
				{
//						avctp_channel_t * chan;
//						chan = avctp_get_channel_for_local_cid(channel);
							avctp_l2cap_packet_handler(channel, packet, size);
//							avctp_try_respond(chan->l2cap_cid);
				}
            break;
        default:
            break;
    }
}


static void avctp_try_respond(){
    if (!avctp_response_size ) return;
    if (!l2cap_cid) return;
    if (!l2cap_can_send_packet_now(l2cap_cid)) return;
    
    // update state before sending packet (avoid getting called when new l2cap credit gets emitted)
    uint16_t size = avctp_response_size;
    avctp_response_size = 0;
    l2cap_send_internal(l2cap_cid, avctp_response_buffer, size);
}

uint8_t AVCTP_API_sendData(uint8_t uLable, uint8_t uCMD, uint8_t *uPara,uint8_t lenth)
{
	
	uint8_t i;
	uint8_t bRetVal = 0;
	uint8_t aData[lenth+3];
	/* Prepare the Frame */
	/* Header */ 
	avctp_response_buffer[0] = (uLable<<4) | (uCMD<<1);
	avctp_response_buffer[1] = 0x11;
	avctp_response_buffer[2] = 0x0e;
	/* information */ 
	for(i=0;i<lenth;i++)
		avctp_response_buffer [i+3] = uPara[i];
	avctp_response_size = lenth + 3;
	avctp_try_respond();
	return bRetVal;


}


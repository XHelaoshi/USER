
/*
 * Implementation of the AUDIO/VIDEO DISTRIBUTION
 * TRANSPORT PROTOCOL SPECIFICATION
 */

#include "avdtp.h"
#include "btstack_memory.h"
#include <btstack/hci_cmds.h>

#include <stdio.h>
#include <string.h>
#include "hci_dump.h"
#include "l2cap.h"

#include "debug.h"

#define AVDTP_RESPONSE_BUFFER_SIZE (HCI_ACL_BUFFER_SIZE-HCI_ACL_HEADER_SIZE)

static void avdtp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);


//static uint16_t avdtp_cid = 0;
//static uint16_t avdtp_response_size = 0;

//static avdtp_signaling_response_t signaling_responses[NR_AVDTP_PENDING_SIGNALING_RESPONSES];
//static int signaling_responses_pending;
avdtp_signaling_response_t avdtp_signaling_respons;
//static avdtp_channel_t avdtp_channl;
static uint8_t StreamOpened = 0;

static linked_list_t avdtp_channels;
static uint8_t avdtp_response_buffer[AVDTP_RESPONSE_BUFFER_SIZE];

static void (*app_packet_handler)(void * connection, uint8_t packet_type,
                                  uint16_t channel, uint8_t *packet, uint16_t size);
static void (*dreamer_packet_handler)(void * connection, uint8_t packet_type,
                                  uint16_t channel, uint8_t *packet, uint16_t size);
static void avdtp_try_respond(uint16_t avdtp_cid);

void avdtp_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type,
                                                    uint16_t channel, uint8_t *packet, uint16_t size)){
	app_packet_handler = handler;
}
																										
void dreamer_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type,
                                                    uint16_t channel, uint8_t *packet, uint16_t size)){
	dreamer_packet_handler = handler;
}
																										
void avdtp_signalling_handler( uint8_t packet_type,
                                  uint16_t channel, uint8_t *packet, uint16_t size){
	(*app_packet_handler)(NULL, packet_type, channel, packet, size);
}
																	
void avdtp_stream_handler( uint8_t packet_type,
                                  uint16_t channel, uint8_t *packet, uint16_t size){
		(*dreamer_packet_handler)(NULL, packet_type, channel, packet, size);
																	}

void avdtp_init(uint8_t acp_seid){
    // register with l2cap psm sevices - max MTU
    l2cap_register_service_internal(NULL, avdtp_packet_handler, PSM_AVDTP, 0x03F0, LEVEL_0);
//		signaling_responses_pending = 0;
//	avdtp_channl.state = AVDTP_State_Idle;
//	avdtp_channl.avdtp_signaling.acp_seid = acp_seid;
	
}

avdtp_channel_t * avdtp_get_channel_for_local_cid(uint16_t local_cid){
    linked_list_iterator_t it;    
    linked_list_iterator_init(&it, &avdtp_channels);
    while (linked_list_iterator_has_next(&it)){
        avdtp_channel_t * channel = (avdtp_channel_t *) linked_list_iterator_next(&it);
        if ( channel->l2cap_cid == local_cid) {
            return channel;
        }
    } 
    return NULL;
}

void avdtp_accept(uint16_t avdtp_cid, uint8_t *data, uint16_t len){
		avdtp_channel_t *chan = avdtp_get_channel_for_local_cid(avdtp_cid);
		avdtp_response_buffer[0] = (avdtp_signaling_respons.transaction_label << 4) |\
													AVDTP_PKT_TYPE_SINGLE | AVDTP_MESSAGE_ACCEPT;
		avdtp_response_buffer[1] = avdtp_signaling_respons.signal_identifier;
		for(uint8_t i=0;i<len;i++)
		{
			avdtp_response_buffer[2+i] = data[i];	
		}
		chan->avdtp_response_size = len + 2;
}

static void avdtp_hci_event_handler(uint16_t channel,uint8_t *packet, uint16_t size){
	
			switch (packet[0]) {
				case L2CAP_EVENT_INCOMING_CONNECTION:
				{
					avdtp_channel_t * chan;
					chan	= btstack_memory_avdtp_channel_get();
					if (!chan) {
							// CONNECTION REJECTED DUE TO LIMITED RESOURCES 
							l2cap_decline_connection_internal(channel, 0x04);
							break;
					}
					memset(chan, 0, sizeof(avdtp_channel_t));
					// accept
					chan->l2cap_cid = channel;
					chan->avdtp_response_size = 0;
					chan->remote_mtu = l2cap_get_remote_mtu_for_local_cid(channel);
					if(!StreamOpened){
						StreamOpened = 1;
						chan->channel_type = AVDTP_TYPE_SIGNALLING;
						chan->packet_handler = avdtp_signalling_handler;
					}
					else
					{
						chan->channel_type = AVDTP_TYPE_STREAM;
						chan->packet_handler = avdtp_stream_handler;
					}
					// add to connections list
					linked_list_add(&avdtp_channels, (linked_item_t *) chan);					
					l2cap_accept_connection_internal(channel);
									
					break;
				}
				case L2CAP_EVENT_CHANNEL_OPENED:
						if (packet[2]) {
								// open failed -> reset
//								avdtp_cid = 0;
						}
						break;

                case L2CAP_EVENT_CREDITS:
                case DAEMON_EVENT_HCI_PACKET_SENT:
                    avdtp_try_respond(channel);
                    break;
//                
				case L2CAP_EVENT_CHANNEL_CLOSED:
				{
					avdtp_channel_t * chan;
					StreamOpened = 0;
					chan = avdtp_get_channel_for_local_cid(channel);
					chan->avdtp_response_size = 0;
					linked_list_remove(&avdtp_channels, (linked_item_t *) chan);
					btstack_memory_avdtp_channel_free(chan);
				}
						break;
					                    
				default:
					// other event
					break;
			}
}

static void avdtp_l2cap_packet_handler(avdtp_channel_t * chan, uint8_t *packet, uint16_t size){



  avdtp_signaling_respons.transaction_label = packet[0] >> 4;
	avdtp_signaling_respons.packet_type = (packet[0]>>2)&0x03;
	avdtp_signaling_respons.message_type = packet[0]&0x03;
	avdtp_signaling_respons.signal_identifier = packet[1]&0x3f;
	avdtp_signaling_respons.remote_mtu = l2cap_get_remote_mtu_for_local_cid(chan->l2cap_cid);

	chan->packet_handler(AVDTP_SIGNAL_PACKET, chan->l2cap_cid, packet, size);

}

// we assume that we don't get two requests in a row
static void avdtp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){

    switch (packet_type) {
        case HCI_EVENT_PACKET:
							avdtp_hci_event_handler(channel, packet, size);
							
            break;
        case L2CAP_DATA_PACKET:
				{
						avdtp_channel_t * chan;
						chan = avdtp_get_channel_for_local_cid(channel);
							avdtp_l2cap_packet_handler(chan, packet, size);
							avdtp_try_respond(chan->l2cap_cid);
				}
            break;
        default:
            break;
    }


}

static void avdtp_try_respond(uint16_t avdtp_cid){
		avdtp_channel_t *chan = avdtp_get_channel_for_local_cid(avdtp_cid);
    if (!chan->avdtp_response_size ) return;
    if (!avdtp_cid) return;
    if (!l2cap_can_send_packet_now(avdtp_cid)) return;
    
    // update state before sending packet (avoid getting called when new l2cap credit gets emitted)
    uint16_t size = chan->avdtp_response_size;
    chan->avdtp_response_size = 0;
    l2cap_send_internal(avdtp_cid, avdtp_response_buffer, size);
}


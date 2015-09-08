#ifndef _AVDTP_H_
#define _AVDTP_H_

#include <stdint.h>
#include "btstack-config.h"
#include "btstack.h"

#include <btstack/linked_list.h>

typedef enum {
  AVDTP_Invalid = 0,
	AVDTP_Discover = 1,
	AVDTP_Get_Capabilities,
	AVDTP_Set_Configuration,
	AVDTP_Get_Configuration,
	AVDTP_Reconfigure,
	AVDTP_Open,
	AVDTP_Start,
	AVDTP_Close,
	AVDTP_Suspend,
	AVDTP_abort,
	AVDTP_Security_Control
} AVDTP_PDU_ID_t;

typedef enum {
    AVDTP_State_Idle = 0,
    AVDTP_State_Configured = 1,
		AVDTP_State_Open,
		AVDTP_State_Streaming,
		AVDTP_State_Closing,
		AVDTP_State_Aborting
} AVDTP_STATE_t;

#define AVDTP_TYPE_SIGNALLING 0x00
#define AVDTP_TYPE_STREAM			0x01

#define AVDTP_MESSAGE_CMD	0x00
#define AVDTP_MESSAGE_RFD	0x01
#define AVDTP_MESSAGE_ACCEPT	0x02
#define AVDTP_MESSAGE_REJECT	0x03

#define AVDTP_PKT_TYPE_SINGLE			0x00
#define AVDTP_PKT_TYPE_START			0x01
#define AVDTP_PKT_TYPE_CONTINUE			0x02
#define AVDTP_PKT_TYPE_END			0x03


/* SEP capability categories */
#define AVDTP_MEDIA_TRANSPORT			0x01
#define AVDTP_REPORTING				0x02
#define AVDTP_RECOVERY				0x03
#define AVDTP_CONTENT_PROTECTION		0x04
#define AVDTP_HEADER_COMPRESSION		0x05
#define AVDTP_MULTIPLEXING			0x06
#define AVDTP_MEDIA_CODEC			0x07
#define AVDTP_DELAY_REPORTING			0x08
#define AVDTP_ERRNO				0xff

/* AVDTP error definitions */
#define AVDTP_BAD_HEADER_FORMAT			0x01
#define AVDTP_BAD_LENGTH			0x11
#define AVDTP_BAD_ACP_SEID			0x12
#define AVDTP_SEP_IN_USE			0x13
#define AVDTP_SEP_NOT_IN_USE			0x14
#define AVDTP_BAD_SERV_CATEGORY			0x17
#define AVDTP_BAD_PAYLOAD_FORMAT		0x18
#define AVDTP_NOT_SUPPORTED_COMMAND		0x19
#define AVDTP_INVALID_CAPABILITIES		0x1A
#define AVDTP_BAD_RECOVERY_TYPE			0x22
#define AVDTP_BAD_MEDIA_TRANSPORT_FORMAT	0x23
#define AVDTP_BAD_RECOVERY_FORMAT		0x25
#define AVDTP_BAD_ROHC_FORMAT			0x26
#define AVDTP_BAD_CP_FORMAT			0x27
#define AVDTP_BAD_MULTIPLEXING_FORMAT		0x28
#define AVDTP_UNSUPPORTED_CONFIGURATION		0x29
#define AVDTP_BAD_STATE				0x31

/* SEP types definitions */
#define AVDTP_SEP_TYPE_SOURCE			0x00
#define AVDTP_SEP_TYPE_SINK			0x01

/* Media types definitions */
#define AVDTP_MEDIA_TYPE_AUDIO			0x00
#define AVDTP_MEDIA_TYPE_VIDEO			0x01
#define AVDTP_MEDIA_TYPE_MULTIMEDIA		0x02

#define NR_AVDTP_PENDING_SIGNALING_RESPONSES 0x03

typedef struct avdtp_signaling_response {
    uint8_t  transaction_label;
    uint8_t  packet_type;
    uint8_t message_type; 
		uint16_t signal_identifier;
		uint16_t remote_mtu;
		uint8_t state;
} avdtp_signaling_response_t;

typedef struct avdtp_signaling_channel {
		uint8_t acp_seid;
		uint8_t int_seid;
		avdtp_signaling_response_t avdtp_response_header;
}avdtp_signaling_channel_t;

typedef struct avdtp_channel {
		AVDTP_STATE_t state;
		avdtp_signaling_channel_t avdtp_signaling;
		uint16_t l2cap_cid;	
    uint16_t remote_mtu;
		uint16_t avdtp_response_size;
		uint16_t channel_type;
		btstack_packet_handler_t packet_handler;
}avdtp_channel_t;

void avdtp_init(uint8_t acp_seid);
void avdtp_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type,
                                                    uint16_t channel, uint8_t *packet, uint16_t size));
void dreamer_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type,
                                                    uint16_t channel, uint8_t *packet, uint16_t size));
int avdtp_send_internal(uint16_t avdtp_cid, uint8_t *data, uint16_t len);
void avdtp_accept(uint16_t avdtp_cid, uint8_t *data, uint16_t len);


#endif


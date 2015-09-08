#ifndef _AVRCP_H_
#define _AVRCP_H_

#include <stdint.h>
#include "btstack-config.h"
#include "btstack.h"

#include <btstack/linked_list.h>

#define AVR_LENGTH 5
#define META_LENGTH 11

#define CMD_C	0

/* opcodes */
#define AVR_OP_VENDORDEP		0x00
#define AVR_OP_UNITINFO			0x30
#define AVR_OP_SUBUNITINFO		0x31
#define AVR_OP_PASSTHROUGH		0x7c

/* subunits of interest */
#define AVR_SUBUNIT_PANEL		0x09

/* operands in passthrough commands */
#define AVR_SELECT			0x00
#define AVR_UP				0x01
#define AVR_DOWN			0x02
#define AVR_LEFT			0x03
#define AVR_RIGHT			0x04
#define AVR_ROOT_MENU			0x09
#define AVR_CONTENTS_MENU		0x0b
#define AVR_FAVORITE_MENU		0x0c
#define AVR_ENTER			0x2b
#define AVR_CHANNEL_UP			0x30
#define AVR_CHANNEL_DOWN		0x31
#define AVR_INPUT_SELECT		0x34
#define AVR_HELP			0x36
#define AVR_POWER			0x40
#define AVR_VOLUME_UP			0x41
#define AVR_VOLUME_DOWN			0x42
#define AVR_MUTE			0x43
#define AVR_PLAY			0x44
#define AVR_STOP			0x45
#define AVR_PAUSE			0x46
#define AVR_RECORD			0x47
#define AVR_REWIND			0x48
#define AVR_FAST_FORWARD		0x49
#define AVR_EJECT			0x4a
#define AVR_FORWARD			0x4b
#define AVR_BACKWARD			0x4c
#define AVR_F1				0x71
#define AVR_F2				0x72
#define AVR_F3				0x73
#define AVR_F4				0x74

/* Company IDs for vendor dependent commands */
#define IEEEID_BTSIG		0x001958


/* Status codes */
#define AVRCP_STATUS_INVALID_COMMAND		0x00
#define AVRCP_STATUS_INVALID_PARAM		0x01
#define AVRCP_STATUS_PARAM_NOT_FOUND		0x02
#define AVRCP_STATUS_INTERNAL_ERROR		0x03
#define AVRCP_STATUS_SUCCESS			0x04
#define AVRCP_STATUS_OUT_OF_BOUNDS		0x0b
#define AVRCP_STATUS_INVALID_PLAYER_ID		0x11
#define AVRCP_STATUS_PLAYER_NOT_BROWSABLE	0x12
#define AVRCP_STATUS_NO_AVAILABLE_PLAYERS	0x15
#define AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED	0x16

/* Packet types */
#define AVRCP_PACKET_TYPE_SINGLE	0x00
#define AVRCP_PACKET_TYPE_START		0x01
#define AVRCP_PACKET_TYPE_CONTINUING	0x02
#define AVRCP_PACKET_TYPE_END		0x03

/* PDU types for metadata transfer */
#define AVRCP_GET_CAPABILITIES		0x10
#define AVRCP_LIST_PLAYER_ATTRIBUTES	0X11
#define AVRCP_LIST_PLAYER_VALUES	0x12
#define AVRCP_GET_CURRENT_PLAYER_VALUE	0x13
#define AVRCP_SET_PLAYER_VALUE		0x14
#define AVRCP_GET_PLAYER_ATTRIBUTE_TEXT	0x15
#define AVRCP_GET_PLAYER_VALUE_TEXT	0x16
#define AVRCP_DISPLAYABLE_CHARSET	0x17
#define AVRCP_CT_BATTERY_STATUS		0x18
#define AVRCP_GET_ELEMENT_ATTRIBUTES	0x20
#define AVRCP_GET_PLAY_STATUS		0x30
#define AVRCP_REGISTER_NOTIFICATION	0x31
#define AVRCP_REQUEST_CONTINUING	0x40
#define AVRCP_ABORT_CONTINUING		0x41
#define AVRCP_SET_ABSOLUTE_VOLUME	0x50
#define AVRCP_SET_BROWSED_PLAYER	0x70
#define AVRCP_GET_FOLDER_ITEMS		0x71
#define AVRCP_CHANGE_PATH		0x72
#define AVRCP_GET_ITEM_ATTRIBUTES	0x73
#define AVRCP_PLAY_ITEM			0x74
#define AVRCP_SEARCH			0x80
#define AVRCP_ADD_TO_NOW_PLAYING	0x90
#define AVRCP_GENERAL_REJECT		0xA0


/* Notification events */
#define AVRCP_EVENT_STATUS_CHANGED		0x01
#define AVRCP_EVENT_TRACK_CHANGED		0x02
#define AVRCP_EVENT_TRACK_REACHED_END		0x03
#define AVRCP_EVENT_TRACK_REACHED_START		0x04
#define AVRCP_EVENT_SETTINGS_CHANGED		0x08
#define AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED	0x0a
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED	0x0b
#define AVRCP_EVENT_UIDS_CHANGED		0x0c
#define AVRCP_EVENT_VOLUME_CHANGED		0x0d
#define AVRCP_EVENT_LAST			AVRCP_EVENT_VOLUME_CHANGED


#define AVRCP_HEADER_LENGTH  0x07

struct avrcp_header {
	uint8_t company_id[3];
	uint8_t pdu_id;
	uint8_t packet_type:2;
	uint8_t rsvd:6;
	uint16_t params_len;
	uint8_t params[1];
};

void avrcp_init(void);


#endif
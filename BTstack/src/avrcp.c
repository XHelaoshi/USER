#include "avrcp.h"
#include "avctp.h"

#include "btstack_memory.h"
#include <btstack/hci_cmds.h>

#include <stdio.h>
#include <string.h>
#include "hci_dump.h"
#include "l2cap.h"

#include "debug.h"

static uint8_t avctp_inited=0;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){

}


void avctp_packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    packet_handler(packet_type, channel, packet, size);
}

void avrcp_init(){
	
	avctp_init();
	avctp_register_packet_handler(avctp_packet_handler);
	avctp_inited = 1;
	
}


/*
 * set_company_id:
 *
 * Set three-byte Company_ID into outgoing AVRCP message
 */
static void set_company_id(uint8_t cid[3], const uint32_t cid_in)
{
	cid[0] = cid_in >> 16;
	cid[1] = cid_in >> 8;
	cid[2] = cid_in;
}

uint8_t AVRCP_press(uint8_t op)
{
	uint8_t uLable,uCMD,lenth;
	uint8_t uPara[AVR_LENGTH];
	
	uLable = 0xb;
	uCMD	 = CMD_C;
	uPara[0] = 0;
	uPara[1] = 0x48;
	uPara[2] = 0x7C;
	uPara[3] = op;
	uPara[4] = 0;
	
	AVCTP_API_sendData(uLable,uCMD,uPara,AVR_LENGTH);
	
	return 1;
}

uint8_t AVRCP_release(uint8_t op)
{
	uint8_t uLable,uCMD,lenth;
	uint8_t uPara[AVR_LENGTH];
	
	uLable = 0xb;
	uCMD	 = CMD_C;
	uPara[0] = 0;
	uPara[1] = 0x48;
	uPara[2] = 0x7C;
	uPara[3] = 0x80 | op;
	uPara[4] = 0;
	
	AVCTP_API_sendData(uLable,uCMD,uPara,AVR_LENGTH);
	
	return 1;
}

uint8_t Meta_tr(uint8_t pdu_id ,uint8_t len,uint8_t cap_id)
{
	uint8_t uLable,uCMD,lenth;
	uint8_t uPara[META_LENGTH];
	
	uLable = 0xb;
	uCMD	 = CMD_C;
	uPara[0] = 0x01;
	uPara[1] = 0x48;
	uPara[2] = 0x00;
	uPara[3] = 0x00;
	uPara[4] = 0x19;
	uPara[5] = 0x58;
	uPara[6] = pdu_id;
	uPara[7] = 0x00;
	uPara[8] = 0x00;
	uPara[9] = len;
	uPara[10] = cap_id;
	
	AVCTP_API_sendData(uLable,uCMD,uPara,META_LENGTH);
	
	return 1;
}

uint8_t Get_Attrib()
{
	uint8_t uLable,uCMD,lenth;
	uint8_t uPara[27]={0x01,0x48,0x00,0x00,0x19,0x58,0x20,0,0,0x0d,0,0,0,0,0x02,0,0,0,0,0,0,0,0x01,0,0,0,0x07};
	
	uLable = 0xb;
	uCMD	 = CMD_C;
// 	uPara[0] = 0x01;
// 	uPara[1] = 0x48;
// 	uPara[2] = 0x00;
// 	uPara[3] = 0x00;
// 	uPara[4] = 0x19;
// 	uPara[5] = 0x58;
// 	uPara[6] = 0x20;
// 	uPara[7] = 0x00;
// 	uPara[8] = 0x00;
// 	uPara[9] = 0x0d;
// 	uPara[10] = 0;
// 	uPara[11] = 
	
	AVCTP_API_sendData(uLable,uCMD,uPara,27);
	
	return 1;
}


static void get_element()
{
	uint8_t buf[AVRCP_HEADER_LENGTH + 9+3];
	struct avrcp_header *pdu ;
	uint16_t length;
	uint8_t uLable,uCMD,lenth;
	rt_memset(buf, 0, sizeof(buf));

	pdu = (void *) (buf+3);
	uLable = 0xb;
	uCMD	 = CMD_C;
	buf[0] = 0x01;
	buf[1] = 0x48;
	buf[2] = 0x00;
	set_company_id(pdu->company_id, IEEEID_BTSIG);
	pdu->pdu_id = AVRCP_GET_ELEMENT_ATTRIBUTES;
	pdu->params_len = 0x0009;
	pdu->packet_type = AVRCP_PACKET_TYPE_SINGLE;

	length = AVRCP_HEADER_LENGTH +pdu->params_len;
	
	AVCTP_API_sendData(uLable,uCMD,buf,length+3);
}

#include<finsh.h>
FINSH_FUNCTION_EXPORT(AVRCP_press,press);
FINSH_FUNCTION_EXPORT(AVRCP_release,release);
FINSH_FUNCTION_EXPORT(Meta_tr,Metadata_transfer);
FINSH_FUNCTION_EXPORT(Get_Attrib,Get Element Attributes);
FINSH_FUNCTION_EXPORT(get_element,get_element);




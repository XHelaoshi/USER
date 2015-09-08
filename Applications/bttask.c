#include <rtthread.h>
#include <rthw.h>


#include "hci.h"
#include "btstack_memory.h"
#include "run_loop.h"
#include "hci_dump.h"
#include "sdp.h"
#include <btstack/sdp_util.h>
#include "hsp.h"
#include "spp.h"

#include "rda5875.h"
#define memset rt_memset
#define RFCOMM_SERVER_CHANNEL_SPP 4

rt_sem_t nw_bt_sem;
void rt_bt_thread_entry(void * para)
{
	hci_transport_t    * transport;
	hci_uart_config_t * config = NULL;
	bt_control_t       * control   = NULL;
	remote_device_db_t * remote_db;
	
//	static uint8_t   hsp_service_buffer[150];
//	static uint8_t   hfp_service_buffer[150];	
//	static uint8_t   a2dp_service_buffer[150];
//	static uint8_t   avrcp_service_buffer[150];
	//static uint8_t   obex_service_buffer[150];
	static uint8_t   spp_service_buffer[150];

	rt_thread_delay(150);
	RDA5875_Init();
	
	nw_bt_sem = rt_sem_create("bt_sem", 1, RT_IPC_FLAG_PRIO);
	
	
	
	// start with BTstack init - especially configure HCI Transport
	hci_dump_open(NULL, HCI_DUMP_STDOUT);
	btstack_memory_init();
	run_loop_init(RUN_LOOP_EMBEDDED);
	
    // init HCI
	transport = hci_transport_h4_dma_instance();
  remote_db = (remote_device_db_t *) &remote_device_db_memory;
	hci_init(transport, config, control, remote_db);
	
	gap_set_local_name("hhy_bt");
	hci_set_class_of_device(0x24040c);
	
	hci_discoverable_control (1) ;
	l2cap_init();
	rfcomm_init();
//	rfcomm_register_packet_handler(hsp_rfcomm_packet_handler);
	//rfcomm_register_packet_handler(obex_rfcomm_packet_handler);
	rfcomm_register_packet_handler(spp_rfcomm_packet_handler);
	//rfcomm_register_packet_handler(spp_rfcomm_packet_handler1);
	
//	rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL_HFP, 150);
//	rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL_HSP, 150);
	rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL_SPP, 150);
	sdp_init();



//	memset(hsp_service_buffer, 0, sizeof(hsp_service_buffer));
//	service_record_item_t * service_hsp_record_item = (service_record_item_t *) hsp_service_buffer;
//	sdp_create_HSP_service( (uint8_t*) &service_hsp_record_item->service_record, RFCOMM_SERVER_CHANNEL_HSP, "HSP");
//	sdp_register_service_internal(NULL, service_hsp_record_item);
//	
	///*
//		OBEX add in SDP
//	*/
//	memset(obex_service_buffer, 0, sizeof(obex_service_buffer));
//	service_record_item_t * service_obex_record_item = (service_record_item_t *) obex_service_buffer;
//	sdp_create_OBEX_service( (uint8_t*) &service_obex_record_item->service_record, RFCOMM_SERVER_CHANNEL_OBEX, "OBEX");
//	sdp_register_service_internal(NULL, service_obex_record_item);	
	//spp in sdp
	memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
	service_record_item_t * service_spp_record_item = (service_record_item_t *) spp_service_buffer;
	sdp_create_spp_service( (uint8_t*) &service_spp_record_item->service_record, RFCOMM_SERVER_CHANNEL_SPP, "SPP_COM5");
	sdp_register_service_internal(NULL, service_spp_record_item);
	
	
//	memset(hfp_service_buffer, 0, sizeof(hfp_service_buffer));
//	service_record_item_t * service_hfp_record_item = (service_record_item_t *) hfp_service_buffer;
//	sdp_create_HFP_service( (uint8_t*) &service_hfp_record_item->service_record, RFCOMM_SERVER_CHANNEL_HFP, "HFP");
//	sdp_register_service_internal(NULL, service_hfp_record_item);
//	
//	memset(a2dp_service_buffer, 0, sizeof(a2dp_service_buffer));
//	service_record_item_t * service_a2dp_record_item = (service_record_item_t *) a2dp_service_buffer;
//	sdp_create_A2DP_service( (uint8_t*) &service_a2dp_record_item->service_record, 0, "A2DP");
//	sdp_register_service_internal(NULL, service_a2dp_record_item);

//	memset(avrcp_service_buffer, 0, sizeof(avrcp_service_buffer));
//	service_record_item_t * service_avrcp_record_item = (service_record_item_t *) avrcp_service_buffer;
//	sdp_create_AVRCP_service( (uint8_t*) &service_avrcp_record_item->service_record, 0, "AVRCP");
//	sdp_register_service_internal(NULL, service_avrcp_record_item);
	
///*
//		OBEX add in SDP
//	*/
//	memset(obex_service_buffer, 0, sizeof(obex_service_buffer));
//	service_record_item_t * service_obex_record_item = (service_record_item_t *) obex_service_buffer;
//	sdp_create_OBEX_service( (uint8_t*) &service_obex_record_item->service_record, RFCOMM_SERVER_CHANNEL_OBEX, "OBEX");
//	sdp_register_service_internal(NULL, service_obex_record_item);	
//	
//	a2dp_init();
//	avrcp_init();
//	
	
	hci_power_control(HCI_POWER_ON);
	rt_thread_delay(100);
	run_loop_execute();
}
/*
		OBEX add in SDP
	*/
//	memset(obex_service_buffer, 0, sizeof(obex_service_buffer));
//	service_record_item_t * service_obex_record_item = (service_record_item_t *) obex_service_buffer;
//	sdp_create_OBEX_service( (uint8_t*) &service_obex_record_item->service_record, RFCOMM_SERVER_CHANNEL_OBEX, "OBEX");
//	sdp_register_service_internal(NULL, service_obex_record_item);

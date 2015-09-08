#ifndef _AVCTP_H_
#define _AVCTP_H_

#include <stdint.h>
#include "btstack-config.h"
#include "btstack.h"

#include <btstack/linked_list.h>

void avctp_init(void);
void avctp_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type,
                                                    uint16_t channel, uint8_t *packet, uint16_t size));
uint8_t AVCTP_API_sendData(uint8_t uLable, uint8_t uCMD, uint8_t *uPara,uint8_t lenth);

#endif

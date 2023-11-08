#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H
#include "sdmessage.pb-c.h"
#include "table.h"

uint8_t *serialize_message(MessageT *message);

MessageT *deserialize_message(uint8_t *message_data, size_t message_size);

int invoke(MessageT *received_msg, struct table_t *table);

#endif
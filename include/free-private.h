#ifndef _FREE_PRIVATE_H
#define _FREE_PRIVATE_H

#include "client_stub.h"
#include "sdmessage.pb-c.h"

int free_rtable_t(rtable_t *rtable);

int free_MessageT(MessageT *message);

int free_EntryT(EntryT *entry);

#endif
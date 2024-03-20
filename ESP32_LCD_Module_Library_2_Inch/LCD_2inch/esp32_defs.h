#ifndef _ESP_DEF_H_
#define _ESP_DEF_H_

#define ICACHE_RODATA_ATTR  __attribute__((section(".irom.text")))
#define PROGMEM   ICACHE_RODATA_ATTR

#endif

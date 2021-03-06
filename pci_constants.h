typedef unsigned char byte;
typedef unsigned int uint32;

#define NOT_FOUND "Not found."

#define NO_DEVICE 0xffffffff

#define CONTROL_PORT 0x0CF8
#define INFO_PORT 0x0CFC

#define BUS_SHIFT 16
#define DEVICE_SHIFT 11
#define FUNCTION_SHIFT 8
#define REGISTER_SHIFT 2
#define CONTROL_BIT (1 << 31)

#define IDS_REGISTER 0
#define CLASS_CODE_REGISTER 2

#define HEADER_TYPE_REGISTER 3
#define CACHE_LINE_SIZE_REGISTER 3

#define BUS_NUMBERS_REGISTER 6
#define IO_REGISTER 7
#define MEMORY_REGISTER 8

#define EXPANSION_ROM_REGISTER 12

#define INTERRPUT_PIN_REGISTER 15
#define INTERRPUT_LINE_REGISTER 15

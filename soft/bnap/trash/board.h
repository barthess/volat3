#ifndef BOARD_H 
#define BOARD_H
//==============================================================================
//                                        
//==============================================================================

#include "AT91SAM7A3.h"
//------------------------------------------------------------------------------

// Частота кварцевого генератора
#define BOARD_MAINOSC           18432000

// Master clock frequency.
#define BOARD_MCK               47923200L
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Pin types 
//------------------------------------------------------------------------------
// The pin is controlled by the associated signal of peripheral A.
#define PIO_PERIPH_A                0
// The pin is controlled by the associated signal of peripheral B.
#define PIO_PERIPH_B                1
// The pin is an input.
#define PIO_INPUT                   2
// The pin is an output and has a default level of 0.
#define PIO_OUTPUT_0                3
// The pin is an output and has a default level of 1.
#define PIO_OUTPUT_1                4
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Pin attributes
//------------------------------------------------------------------------------
/// Default pin configuration (no attribute).
#define PIO_DEFAULT                 (0 << 0)
/// The internal pin pull-up is active.
#define PIO_PULLUP                  (1 << 0)
/// The internal glitch filter is active.
#define PIO_DEGLITCH                (1 << 1)
/// The pin is open-drain.
#define PIO_OPENDRAIN               (1 << 2)
//------------------------------------------------------------------------------

/// Calculates the size of a Pin instances array. The array must be local (i.e.
/// not a pointer), otherwise the computation will not be correct.
#define PIO_LISTSIZE(list)    (sizeof(list) / sizeof(Pin))

//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------
/// Describes the type and attribute of one PIO pin or a group of similar pins.
typedef struct {
    /// Bitmask indicating which pin(s) to configure.
    unsigned int mask; 
    /// Pointer to the PIO controller which has the pin(s).
    AT91S_PIO    *pio;
    /// Peripheral ID of the PIO controller which has the pin(s).
    unsigned char id;
    /// Pin type (see "Pin types").
    unsigned char type;
    /// Pin attribute (see "Pin attributes").
    unsigned char attribute;
} Pin;
//------------------------------------------------------------------------------

// Base address of the MCI peripheral connected to the SD card.
#define BOARD_SD_MCI_BASE           AT91C_BASE_MCI
// Peripheral identifier of the MCI connected to the SD card.
#define BOARD_SD_MCI_ID             AT91C_ID_MCI
// MCI slot to which the SD card is connected to.
#define BOARD_SD_SLOT               MCI_SD_SLOTA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
// Определения для USB
//------------------------------------------------------------------------------
// Используется USB интерфейс
#define BOARD_USB_UDP

// Indicates the D+ pull-up is external.
#define BOARD_USB_PULLUP_EXTERNAL

// Number of endpoints in the USB controller.
#define BOARD_USB_NUMENDPOINTS                  6

// Returns the maximum packet size of the given endpoint.
#define BOARD_USB_ENDPOINTS_MAXPACKETSIZE(i)    ((i == 0) ? 8 : ((i == 4 || i == 5) ? 512 : 64))

// Returns the number of FIFO banks for the given endpoint.
#define BOARD_USB_ENDPOINTS_BANKS(i)            (((i == 0) || (i == 3)) ? 1 : 2)

// USB attributes configuration descriptor (bus or self powered, remote wakeup)
#define BOARD_USB_BMATTRIBUTES                  USBConfigurationDescriptor_BUSPOWERED_NORWAKEUP
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Подключение GPS приемника
//------------------------------------------------------------------------------
#define PINS_UARTD_RXD_TXD {AT91C_PA30_DRXD | AT91C_PA31_DTXD, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_UARTD          PINS_UARTD_RXD_TXD
#define PIN_GPS_RESET      {AT91C_PIO_PA19, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_OPENDRAIN}

//------------------------------------------------------------------------------
// Подключение Crypto-карты
// и МПиОВДа
//------------------------------------------------------------------------------
#define PINS_USART0_RXD_TXD {AT91C_PA2_RXD0 | AT91C_PA3_TXD0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_USART0_RTS_CTS {AT91C_PA5_RTS0 | AT91C_PA6_CTS0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_SCK      {AT91C_PA4_SCK0, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_USART0         PINS_USART0_RXD_TXD

//------------------------------------------------------------------------------
// Подключение GSM модема
//------------------------------------------------------------------------------
// Управление питанием 1 - модем включен
#define PIN_GSM_ON_OFF       {AT91C_PIO_PA24, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}   
#define PIN_GSM_RESET        {AT91C_PIO_PA18, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_OPENDRAIN}
#define PINS_USART1_RXD_TXD  {AT91C_PA7_RXD1 | AT91C_PA8_TXD1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_USART1_RTS_CTS  {AT91C_PB24_RTS1 | AT91C_PB25_CTS1, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_USART1_SCK       {AT91C_PB26_SCK1, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}
#define PINS_USART1          PINS_USART1_RXD_TXD, PINS_USART1_RTS_CTS

//------------------------------------------------------------------------------
// Подключение графической панели
// Оно же модуль отобрАжения
//------------------------------------------------------------------------------
#define PIN_CS_LCD           {AT91C_PIO_PB9, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}
#define PINS_USART2_RXD_TXD  {AT91C_PA9_RXD2 | AT91C_PA10_TXD2, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_USART2          PINS_USART2_RXD_TXD
//#define PINS_USART2_RTS_CTS  {AT91C_PB27_RTS2 | AT91C_PB28_CTS2, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}

//------------------------------------------------------------------------------
// Интерфейса TWI
//------------------------------------------------------------------------------
#define PINS_TWD  {AT91C_PA0_TWD,  AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
#define PINS_TWC  {AT91C_PA1_TWCK, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
#define PINS_TWI  PINS_TWD, PINS_TWC

//------------------------------------------------------------------------------
// Цифровые входы
//------------------------------------------------------------------------------
#define D_IN0 AT91C_PIO_PB4 
#define D_IN1 AT91C_PIO_PB5 
#define PINS_INPUTD {D_IN0 | D_IN1 , AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT} 

//------------------------------------------------------------------------------
// Клавиатура
//------------------------------------------------------------------------------
#define KBD_ALARM  AT91C_PIO_PB19
#define KBD_ENTER  AT91C_PIO_PB20
#define KBD_ESC    AT91C_PIO_PB21
#define KBD_UP     AT91C_PIO_PB22
#define KBD_DOWN   AT91C_PIO_PB23
#define PINS_KBD   {KBD_ALARM | KBD_UP | KBD_DOWN | KBD_ENTER | KBD_ESC, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}

//------------------------------------------------------------------------------
// Цифровые выходы
//------------------------------------------------------------------------------
#define PIN_OTPUT0 {AT91C_PIO_PA21, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT} 
#define PIN_OTPUT1 {AT91C_PIO_PA23, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT} 

//------------------------------------------------------------------------------
// Строб
//------------------------------------------------------------------------------
//#define PIN_STROB {AT91C_PIO_, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}

//------------------------------------------------------------------------------
// Аналоговые входы
//------------------------------------------------------------------------------
#define ANALOG_0 0
#define ANALOG_1 1
#define ANALOG_2 2
//------------------------------------------------------------------------------

// Микрофонный усилитель MAX9814
//------------------------------------------------------------------------------
#define PIN_MAX9814_GAIN AT91C_PIO_PB7
#define PIN_MAX9814_AR   AT91C_PIO_PB6
#define PIN_MAX9814_SD   AT91C_PIO_PB8
//------------------------------------------------------------------------------

// Выходной усилитель TDA8551
//------------------------------------------------------------------------------
#define PIN_TDA8551_GAIN AT91C_PIO_PB17
#define PIN_TDA8551_MODE AT91C_PIO_PB18
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Определения для MMC
//------------------------------------------------------------------------------
#define MMC_SPI_DI    AT91C_PA15_MCDA0  
#define MMC_SPI_SCLK  AT91C_PA17_MCCK
#define MMC_SPI_DO    AT91C_PA16_MCCDA
#define MMC_SPI_CS    AT91C_PA14_MCDA3
#define MMC_CDT       AT91C_PIO_PA28
#define MMC_WPT       AT91C_PIO_PA29

#define PIN_MMC_SPI_DI    {MMC_SPI_DI, AT91C_BASE_PIOB, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT}  
#define PIN_MMC_SPI_SCLK  {MMC_SPI_SCLK,  AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_MMC_SPI_DO    {MMC_SPI_DO, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_MMC_SPI_CS    {MMC_SPI_CS, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_MMC_WPT       {MMC_WPT,    AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_MMC_CDT       {MMC_CDT,    AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PINS_MMC          PIN_MMC_SPI_DI, PIN_MMC_SPI_SCLK, PIN_MMC_SPI_DO, PIN_MMC_SPI_CS, PIN_MMC_WPT, PIN_MMC_CDT 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif //#ifndef BOARD_H

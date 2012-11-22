#ifndef __XMODEM_H__
#define __XMODEM_H__
/* =============================================================================

    Copyright (c) 2006 Pieter Conradie [www.piconomic.co.za]
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Title:          xmodem.h : XMODEM-CRC receive module
    Author(s):      Pieter Conradie
    Creation Date:  2007-03-31
    Revision Info:  $Id$

============================================================================= */

/**
    @ingroup COMMS
    @defgroup XMODEM xmodem.h : XMODEM-CRC receive module

    Receive or send a file via the XMODEM-CRC protocol.

    File(s):
    - comms/xmodem.h
    - comms/xmodem.c

    XMODEM-CRC is a simple file transfer protocol. The file is broken up into a
    series of packets that are sent to the receiver.

    Each packet consists of:
    - a SOH character
    - a "block number" from 0-255
    - the "inverse" block number
    - 128 bytes of data
    - 16-bit CRC (in big endian order)

    A single character response is sent by the receiver to control the flow of
    packets:
    - 'C' [0x43] : ASCII 'C' to start an XMODEM transfer using CRC
    - ACK [0x06] : Acknowledge
    - NAK [0x15] : Not Acknowledge

    To indicate the end of transfer, the transmitter sends:
    - EOT [0x04] : End of Transfer

    @see http://en.wikipedia.org/wiki/XMODEM

    This module requires a few functions that must be defined externally.
    Macros are used to bind to these functions (for flexibility and optimisation)
    and must be defined in 'config.h', e.g.

        @code
        #include "uart.h"
        #include "tmr.h"

        #define XMODEM_READ_U8(data)        uart_read_u8(data)
        #define XMODEM_WRITE_U8(data)       uart_put_char((char)data)
        #define XMODEM_TMR_START(ms)        tmr_start(TMR_MS_TO_START_VAL(ms))
        #define XMODEM_TMR_HAS_EXPIRED()    tmr_has_expired()
        @endcode
 */
/// @{

/* _____PROJECT INCLUDES_____________________________________________________ */
#include "ch.h"
#include "hal.h"

/* _____DEFINITIONS _________________________________________________________ */
// Verify that binding macros has been defined
#define U16_HI8(crc) (((crc)>>8) & 0xFF)
#define U16_LO8(crc) ((crc) & 0xFF)

#define XMODEM_WRITE_U8(c) sdPut((&(SDDM)), (c))

#ifndef XMODEM_TMR_START
#error "XMODEM_TMR_START() macro must be defined. See documentation in xmodem.h"
#endif
#ifndef XMODEM_TMR_HAS_EXPIRED
#error "XMODEM_TMR_HAS_EXPIRED() macro must be defined. See documentation in xmodem.h"
#endif


/* _____TYPE DEFINITIONS_____________________________________________________ */
/**
    Definition for a pointer to a function that will be called once a block of
    data has been received.
 */
typedef void (*xmodem_on_rx_data_t)(const uint8_t *data, uint8_t bytes_received);

/**
    Definition for a pointer to a function that will be called to supply
    data to send.
 */
typedef bool_t (*xmodem_on_tx_data_t)(uint8_t *data, uint8_t bytes_to_send);

/* _____GLOBAL VARIABLES_____________________________________________________ */

/* _____GLOBAL FUNCTION DECLARATIONS_________________________________________ */
/**
    Blocking function that receives a file using the XMODEM-CRC protocol.

    @param on_rx_data   Pointer to a function that will be called once a block of
                        data has been received.

    @retval TRUE        File succesfully received
    @retval FALSE       Timed out while trying to receive a file
 */
bool_t xmodem_receive_file(xmodem_on_rx_data_t on_rx_data);

/**
    Blocking function that sends a file using the XMODEM-CRC protocol.

    @param on_tx_data   pointer to a function that will be called to supply
                        data to send.

    @retval TRUE        File succesfully sent
    @retval FALSE       Timed out while trying to send a file
 */
bool_t xmodem_send_file(xmodem_on_tx_data_t on_tx_data);

/* _____MACROS_______________________________________________________________ */

/// @}
#endif

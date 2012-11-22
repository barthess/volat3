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

    Title:          XMODEM-CRC receive module
    Author(s):      Pieter Conradie
    Creation Date:  2007-03-31
    Revision Info:  $Id$

============================================================================= */
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "main.h"

#include "xmodem.h"


/* _____LOCAL DEFINITIONS____________________________________________________ */
/// @name XMODEM protocol definitions
//@{
#define XMODEM_DATA_SIZE         128
#define XMODEM_TIMEOUT_MS        1000
#define XMODEM_MAX_RETRIES       4
#define XMODEM_MAX_RETRIES_START 1
//@}

/// @name XMODEM flow control characters
//@{
#define XMODEM_SOH               0x01 ///< Start of Header
#define XMODEM_EOT               0x04 ///< End of Transmission
#define XMODEM_ACK               0x06 ///< Acknowledge
#define XMODEM_NAK               0x15 ///< Not Acknowledge
#define XMODEM_C                 0x43 ///< ASCII C
//@}

// XMODEM packet structure definition
typedef struct
{
    uint8_t  start;
    uint8_t  packet_nr;
    uint8_t  packet_nr_inv;
    uint8_t  data[XMODEM_DATA_SIZE];
    uint8_t  crc16_hi8;
    uint8_t  crc16_lo8;
} xmodem_packet_t;

/* _____LOCAL VARIABLES______________________________________________________ */
// Variable to keep track of current packet number
static uint8_t xmodem_packet_nr;

// Packet buffer
static union
{
    xmodem_packet_t packet;
    uint8_t            data[sizeof(xmodem_packet_t)];
} xmodem_packet;

/**
 *
 */
static bool_t xmodem_wait_rx_char(uint8_t *data)
{
  msg_t c = RDY_RESET;
  c = sdGetTimeout(&SDDM, MS2ST(XMODEM_TIMEOUT_MS));
  if (c >= 0){
    *data = c;
    return TRUE;
  }
  else{
    return FALSE;
  }
}

/**
 *
 */
static uint16_t xmodem_calc_checksum(void)
{
  uint8_t  i;
  uint8_t  j;
  uint8_t  data;
  uint16_t crc = 0x0000;

    // Repeat until all the data has been processed...
    for(i=0; i<XMODEM_DATA_SIZE; i++)
    {
        data = xmodem_packet.packet.data[i];

        // XOR high byte of CRC with 8-bit data
        crc = crc ^ (((uint16_t)data)<<8);

        // Repeat 8 times (for each bit)
        for(j=8; j!=0; j--)
        {
            // Is highest bit set?
            if((crc & (1<<15)) != 0)
            {
                // Shift left and XOR with 0x1021
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                // Shift left
                crc = (crc << 1);
            }
        }
    }
        return crc;
}

/**
 *
 */
static bool_t xmodem_verify_checksum(uint16_t crc){
    // Compare received CRC with calculated value
    if(xmodem_packet.packet.crc16_hi8 != U16_HI8(crc))
      return FALSE;
    if(xmodem_packet.packet.crc16_lo8 != U16_LO8(crc))
      return FALSE;
    return TRUE;
}

/**
    Blocking function with a timeout that tries to receive an XMODEM packet

    @retval TRUE    Packet correctly received
    @retval FALSE   Packet error
 */
static bool_t xmodem_rx_packet(void)
{
  uint8_t  i = 0;
  uint8_t  data;

    // Repeat until whole packet has been received
    for(i=0; i<sizeof(xmodem_packet.data); i++){
        // See if character has been received
        if(!xmodem_wait_rx_char(&data)){
            return FALSE;// Timeout
        }
        // Store received data in buffer
        xmodem_packet.data[i] = data;

        // See if this is the first byte of a packet received (xmodem_packet.packet.start)
        if(i == 0){
            // See if End Of Transmission has been received
            if(data == XMODEM_EOT){
                return TRUE;
            }
        }
    }
    // See if whole packet was received
    if(i != sizeof(xmodem_packet.data))
      return FALSE;
    // See if correct header was received
    if(xmodem_packet.packet.start != XMODEM_SOH)
      return FALSE;
    // Check packet number checksum
    if((xmodem_packet.packet.packet_nr + xmodem_packet.packet.packet_nr_inv) != 255)
      return FALSE;

    // Verify Checksum
    return xmodem_verify_checksum(xmodem_calc_checksum());
}

/**
 *
 */
static void xmodem_tx_packet(void)
{
  uint8_t  i;
  uint16_t crc;

    // Start Of Header
    xmodem_packet.packet.start = XMODEM_SOH;
    // Packet number
    xmodem_packet.packet.packet_nr = xmodem_packet_nr;
    // Inverse packet number
    xmodem_packet.packet.packet_nr_inv = 255 - xmodem_packet_nr;
    // Data already filled in...
    // Checksum
    crc = xmodem_calc_checksum();
    xmodem_packet.packet.crc16_hi8 = U16_HI8(crc);
    xmodem_packet.packet.crc16_hi8 = U16_LO8(crc);

    // Send whole packet
    for(i=0; i<sizeof(xmodem_packet.data); i++)
    {
        XMODEM_WRITE_U8(xmodem_packet.data[i]);
    }
}

/* _____GLOBAL FUNCTIONS_____________________________________________________ */
bool_t xmodem_receive_file(xmodem_on_rx_data_t on_rx_data)
{
  uint8_t retry            = XMODEM_MAX_RETRIES_START;
    bool_t first_ack_sent = FALSE;

    // Reset packet number
    xmodem_packet_nr = 1;

    // Repeat until transfer is finished or error count is exceeded
    while(retry--)
    {
        if(!first_ack_sent)
        {
            // Send initial start character to start transfer (with CRC checking)
            XMODEM_WRITE_U8(XMODEM_C);
        }

        // Try to receive a packet
        if(!xmodem_rx_packet())
        {
            if(first_ack_sent)
            {
                XMODEM_WRITE_U8(XMODEM_NAK);
            }
            continue;
        }
        // End Of Transfer received?
        if(xmodem_packet.packet.start == XMODEM_EOT)
        {
            // Acknowledge EOT
            XMODEM_WRITE_U8(XMODEM_ACK);
            break;
        }
        // Duplicate packet received?
        if(xmodem_packet.packet.packet_nr == (xmodem_packet_nr - 1))
        {
            // Acknowledge packet
            XMODEM_WRITE_U8(XMODEM_ACK);
            continue;
        }
        // Expected packet received?
        if(xmodem_packet.packet.packet_nr != xmodem_packet_nr)
        {
            // NAK packet
            XMODEM_WRITE_U8(XMODEM_NAK);
            continue;
        }
        // Pass received data on to handler
        (*on_rx_data)(&xmodem_packet.packet.data[0], sizeof(xmodem_packet.packet.data));
        // Acknowledge packet
        XMODEM_WRITE_U8(XMODEM_ACK);
        // Next packet
        xmodem_packet_nr++;
        // Reset retry count
        retry = XMODEM_MAX_RETRIES;
        // First ACK sent
        first_ack_sent = TRUE;
    }

    // Too many errors?
    if(retry == 0)
    {
        return FALSE;
    }

    // See if more EOTs are received...
    while(retry--)
    {
        // Wait for a packet
        if(!xmodem_rx_packet())
        {
            break;
        }
        // End Of Transfer received?
        if(xmodem_packet.packet.start == XMODEM_EOT)
        {
            // Acknowledge EOT
            XMODEM_WRITE_U8(XMODEM_ACK);
        }
    }

    return TRUE;
}

bool_t xmodem_send_file(xmodem_on_tx_data_t on_tx_data)
{
  uint8_t retry;
  uint8_t data;

    // Reset packet number
    xmodem_packet_nr = 1;

    // Wait for initial start character to start transfer (with CRC checking)
    XMODEM_TMR_START(15000);
    if(!xmodem_wait_rx_char(&data))
    {
        return FALSE;
    }
    if(data != XMODEM_C)
    {
        return FALSE;
    }

    // Get next data block to send
    while((*on_tx_data)(&xmodem_packet.packet.data[0], sizeof(xmodem_packet.packet.data)))
    {
        // Try sending error packet until error count is exceeded
        for(retry = XMODEM_MAX_RETRIES; retry != 0; retry--)
        {
            // Send packet
            xmodem_tx_packet();
            // Wait for an ACK or NAK
            XMODEM_TMR_START(XMODEM_TIMEOUT_MS);
            if(!xmodem_wait_rx_char(&data))
            {
                continue;
            }
            // Received a NAK. Resend packet
            if(data == XMODEM_NAK)
            {
                continue;
            }
            // Received an ACK. Packet has been correctly received
            if(data == XMODEM_ACK)
            {
                break;
            }
        }
        // See if retry count was exceeded
        if(retry == 0)
        {
            return FALSE;
        }

        // Next packet index
        xmodem_packet_nr++;
    }

    for(retry = XMODEM_MAX_RETRIES; retry != 0; retry--)
    {
        // Send "End Of Transfer"
        XMODEM_WRITE_U8(XMODEM_EOT);
        // Wait for response
        XMODEM_TMR_START(XMODEM_TIMEOUT_MS);
        if(!xmodem_wait_rx_char(&data))
        {
            continue;
        }
        if(data == XMODEM_ACK)
        {
            // File successfully transferred
            return TRUE;
        }
    }
    return FALSE;
}




/**
 *
 */
static WORKING_AREA(CrossFromModemThreadWA, 2048);
static msg_t CrossFromModemThread(void *arg) {
  chRegSetThreadName("xmodem_rx");
  (void)arg;
  uint8_t c;

  while (!chThdShouldTerminate()) {
    c = sdGet(&SDGSM);
    sdPut(&SDDM, c);
  }
  return 0;
}


void XmodemInit(void){
  chThdCreateStatic(CrossFromModemThreadWA,
          sizeof(CrossFromModemThreadWA),
          NORMALPRIO,
          CrossFromModemThread,
          NULL);
}


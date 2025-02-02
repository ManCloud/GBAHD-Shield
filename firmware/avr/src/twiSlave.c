/*
 * twiSlave.c
 *
 *  Created on: 02.01.2022
 *      Author: ManCloud
 */

/* === INCLUDES ============================================================= */
#include "twiSlave.h"

/* === TYPES ================================================================ */

/* === MACROS =============================================================== */

/* === GLOBALS ============================================================== */
uint8_t rx_buff[2];     //two byte buffer
int8_t rx_bytes = 0;

/* === PROTOTYPES =========================================================== */

/* === IMPLEMENTATION ======================================================= */
void twi_init(void)
{
    cli();
    // load address into TWI address register
    TWAR = TWI_ADDR << 1;
    // set the TWCR to enable address matching and enable TWI, clear TWINT, enable TWI interrupt
    TWCR = _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWEN);
    sei();
}

void twi_stop(void)
{
    // clear acknowledge and enable bits
    cli();
    TWCR = 0;
    TWAR = 0;
    sei();
}

bool twi_available(void) {
    return rx_bytes == 2;
}

//returns 0xFF if there is no more data in the buffer
uint16_t twi_read(void) {
//    rx_bytes = 0;
    return rx_buff[1] << 8 | rx_buff[0];
}

//returns 0xFF if there is no more data in the buffer
void twi_flush(void) {
    rx_bytes = 0;
//    return rx_buff[1] << 8 | rx_buff[0];
}

ISR (TWI_vect)
{
    static uint8_t buff[2];
    static uint8_t buff_bytes = 0;
    switch (TW_STATUS)
    {
    // Slave Receiver
    case TW_SR_SLA_ACK: //slave was addressed: prepare to receive data
//        MC_LED_PORT |= _BV(MC_LED_RED_PIN);
        TWCR_ACK;
        buff_bytes = 0;
        break;

    case TW_SR_DATA_ACK: // Databyte received
        if(buff_bytes < sizeof(buff)) {
            buff[buff_bytes++]=TWDR;
            TWCR_ACK;
        } else {
            TWCR_NACK;  //too much data sent
        }

        if(buff_bytes == sizeof(buff)) {
            rx_bytes = buff_bytes;
            memmove(rx_buff, buff, sizeof(buff));
        }
        break;
        //Slave transmitter
    case TW_ST_SLA_ACK:
    case TW_ST_DATA_ACK:
        TWCR_NACK;  //this slave can only receive data and not send some
        break;
    case TW_SR_STOP:
//        MC_LED_PORT &= ~_BV(MC_LED_RED_PIN);
        TWCR_ACK;
        break;
    default:
        TWCR_RESET;
        break;

    }
}


/*!******************************************************************
 * \file main_MORSE.c
 * \brief Sens'it SDK template
 * \author Sens'it Team
 * \copyright Copyright (c) 2018 Sigfox, All Rights Reserved.
 *
 * This file is an empty main template.
 * You can use it as a basis to develop your own firmware.
 *******************************************************************/
/******* INCLUDES **************************************************/
#include "sensit_types.h"
#include "sensit_api.h"
#include "error.h"
#include "button_morse.h"
#include "battery.h"
#include "radio_api.h"
#include "serial.h"
#include "AT_parser.h"


/******* GLOBAL VARIABLES ******************************************/
u8 firmware_version[] = "TEMPLATE";


/*******************************************************************/

int main()
{
    error_t     err;
    button_e    btn[5];
    u16         battery_level;
    u32         letter;
    char        number_char;
    u32         buffer_final[3] = {0, 0, 0};
    u8          payload[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int         size;

    /* Start of initialization */

    /* Configure button */
    SENSIT_API_configure_button(INTERRUPT_BOTH_EGDE);
    /* Configure reed switch */
    SENSIT_API_configure_reed_switch(TRUE, INTERRUPT_BOTH_EGDE);
    /* Init Serial port */
    SERIAL_init();
    /* Initialize Sens'it radio */
    err = RADIO_API_init();
    ERROR_parser(err);
    /* Clear pending interrupt */
    pending_interrupt = 0;
    number_char = 0;
    /* End of initialization */

    while (TRUE)
    {
        /* Execution loop */

        /* Check of battery level */
        BATTERY_handler(&battery_level);

        /* RTC alarm interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_RTC) == INTERRUPT_MASK_RTC)
        {
            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_RTC;
        }

        /* Reed switch interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_REED_SWITCH) == INTERRUPT_MASK_REED_SWITCH)
        {
            /* Reset the device */
            SENSIT_API_reset();
        }

        /* Button interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_BUTTON) == INTERRUPT_MASK_BUTTON)
        {
            letter = 1;
            /* Array of clicks in btn[] */
            size = BUTTON_morse_handler(btn);

            /* Convert and register into a buffer btn[] | 6 bits per character */
            letter = REGISTER_input(btn, letter, size);
            /* Check if 4 long press or the buffer is full 15 char max, to send the message */
            if (letter == 16 || number_char == 15)
            {
                LED_control(1, RGB_WHITE);
                RESIZE_buffer(buffer_final, payload);
                RADIO_API_send_message(RGB_YELLOW, payload, 12, FALSE, NULL);
                number_char = 0;
                INIT_values(buffer_final, payload);
            }
                /* Check if the input is a correct morse character | LED feedback: Green OK, Red NOK*/
            else if (CHECK_input(letter))
            {
                number_char++;
                STORE_input(buffer_final, letter, number_char);
                LED_control(1, RGB_GREEN);
            }
            else
                LED_control(1, RGB_RED);

            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_BUTTON;
        }

        /* Check if all interrupt have been clear */
        if (pending_interrupt == 0)
        {
            /* Wait for Interrupt */
            SENSIT_API_sleep(FALSE);
        }
    } /* End of while */
}

/*******************************************************************/
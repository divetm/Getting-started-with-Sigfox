/*!******************************************************************
 * \file main_COMPASS.c
 * \brief Sens'it turned into a compass
 * \author Matthieu Divet
 * \copyright Copyright (c) 2019 Matthieu Divet, All Rights Reserved.
 *
 * Firmware to turn Sens'it into a compass: will send a compass reading (in degrees) when the button is pushed twice
 *******************************************************************/
/******* INCLUDES **************************************************/
#include "sensit_types.h"
#include "sensit_api.h"
#include "error.h"
#include "button.h"
#include "battery.h"
#include "radio_api.h"
#include "fxos8700.h"
#include "discovery.h"


/******* GLOBAL VARIABLES ******************************************/
u8 firmware_version[] = "COMP_v2.0.0";

/******** DEFINES **************************************************/
#define VIBRATION_THRESHOLD                0x10 /* With 2g range, 3,9 mg threshold */
#define VIBRATION_COUNT                    2

/*******************************************************************/

int main()
{
    error_t err;
    button_e btn;
    u16 battery_level;
    bool send = FALSE;

    /* Discovery payload variable */
    discovery_data_s data = {0};
    discovery_payload_s payload;

    /* Start of initialization */

    /* Configure button */
    SENSIT_API_configure_button(INTERRUPT_BOTH_EGDE);

    /* Initialize Sens'it radio */
    err = RADIO_API_init();
    ERROR_parser(err);

    /* Initialize accelerometer */
    err = FXOS8700_init();
    ERROR_parser(err);

    /* Put accelerometer in transient mode */
    FXOS8700_set_transient_mode(FXOS8700_RANGE_2G, VIBRATION_THRESHOLD, VIBRATION_COUNT);

    /* Clear pending interrupt */
    pending_interrupt = 0;

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

        /* Button interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_BUTTON) == INTERRUPT_MASK_BUTTON)
        {
            /* RGB Led ON during count of button presses */
            SENSIT_API_set_rgb_led(RGB_BLUE);

            /* Count number of presses */
            btn = BUTTON_handler();

            /* RGB Led OFF */
            SENSIT_API_set_rgb_led(RGB_OFF);

            if (btn == BUTTON_TWO_PRESSES)
            {
                /* Set button flag to TRUE */
                data.button = TRUE;

                /* Do a compass measurement */
                err = FXOS8700_read_mag_data(&(data.compass));
                if (err != FXOS8700_ERR_NONE)
                {
                    ERROR_parser(err);
                }
                else
                {
                    /* Set send flag */
                    send = TRUE;
                }
            }

            if (btn == BUTTON_FOUR_PRESSES)
            {
                /* Reset the device */
                SENSIT_API_reset();
            }

            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_BUTTON;
        }

        /* Reed switch interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_REED_SWITCH) == INTERRUPT_MASK_REED_SWITCH)
        {
            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_REED_SWITCH;
        }

        /* Accelerometer interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_FXOS8700) == INTERRUPT_MASK_FXOS8700)
        {
            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_FXOS8700;
        }

        /* Check if we need to send a message */
        if (send == TRUE)
        {
            /* Build the payload */
            DISCOVERY_build_payload(&payload, MODE_COMPASS, &data);

            /* Send the message */
            err = RADIO_API_send_message(0xFF, (u8*)&payload, DISCOVERY_PAYLOAD_SIZE, FALSE, NULL);
            /* Parse the error code */
            ERROR_parser(err);

            /* Clear button flag */
            data.button = FALSE;

            /* Clear send flag */
            send = FALSE;
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

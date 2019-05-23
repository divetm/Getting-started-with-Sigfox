/*!******************************************************************
 * \file main_FRIDGE.c
 * \brief Sens'it SDK template
 * \author Sens'it Team
 * \copyright Copyright (c) 2018 Sigfox, All Rights Reserved.
 *
 * This file is an example of a concrete application for the Sens'it: a fridge controller
 *******************************************************************/
/******* INCLUDES **************************************************/
#include "sensit_types.h"
#include "sensit_api.h"
#include "error.h"
#include "button.h"
#include "battery.h"
#include "radio_api.h"
#include "hts221.h"
#include "fxos8700.h"
#include "discovery.h"

/******** DEFINES **************************************************/
#define MEASUREMENT_PERIOD                 1200 /* Measurement & Message sending period, in second */
#define VIBRATION_THRESHOLD                0x10 /* With 2g range, 3,9 mg threshold */
#define VIBRATION_COUNT                    5


/******* GLOBAL VARIABLES ******************************************/
u8 firmware_version[] = "TEMPLATE";


/*******************************************************************/

int main()
{
    error_t err;
    button_e btn;
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

    /* Initialize temperature & humidity sensor */
    err = HTS221_init();
    ERROR_parser(err);

    /* Initialize accelerometer */
    err = FXOS8700_init();
    ERROR_parser(err);

    /* Put accelerometer in transient mode */
    FXOS8700_set_transient_mode(FXOS8700_RANGE_2G, VIBRATION_THRESHOLD, VIBRATION_COUNT);

    /* Configure reed switch */
    SENSIT_API_configure_reed_switch(TRUE, INTERRUPT_RISING_EGDE);

    /* Initialize RTC alarm timer */
    SENSIT_API_set_rtc_alarm(MEASUREMENT_PERIOD);

    /* Clear pending interrupt */
    pending_interrupt = 0;

    /* End of initialization */

    while (TRUE)
    {
        /* Execution loop */

        /* Check of battery level */
        BATTERY_handler(&(data.battery));

        /* RTC alarm interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_RTC) == INTERRUPT_MASK_RTC)
        {
            /* Do a temperatue & relative humidity measurement */
            err = HTS221_measure(&(data.temperature), &(data.humidity));
            if (err != HTS221_ERR_NONE)
            {
                ERROR_parser(err);
            }
            else
            {
                /* Set send flag */
                send = TRUE;
            }

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
                /* Get reed switch state */
                SENSIT_API_get_reed_switch_state(&(data.magnet));

                /* Set button flag to TRUE */
                data.button = TRUE;

                /* Force a RTC alarm interrupt to do a new measurement */
                pending_interrupt |= INTERRUPT_MASK_RTC;
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
            int seconds = 0;
            /* Get reed switch state */
            SENSIT_API_get_reed_switch_state(&(data.magnet));
            while (seconds<=60 && data.magnet != TRUE)
            {
                SENSIT_API_get_reed_switch_state(&(data.magnet));
                SENSIT_API_wait(1000);
                seconds++;
            }
            if (data.magnet == FALSE)
            {
                /* Force a RTC alarm interrupt to do a new measurement */
                pending_interrupt |= INTERRUPT_MASK_RTC;
            }
            else{
                    /* Increment event counter */
                    data.event_counter++;
            }

            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_REED_SWITCH;
        }

        /* Accelerometer interrupt handler */
        if ((pending_interrupt & INTERRUPT_MASK_FXOS8700) == INTERRUPT_MASK_FXOS8700)
        {
            /* Read transient interrupt register */
            FXOS8700_clear_transient_interrupt(&(data.vibration));
            /* Check if a movement has been detected */
            if (data.vibration == TRUE)
            {
                /* Get reed switch state */
                SENSIT_API_get_reed_switch_state(&(data.magnet));

                /* Force a RTC alarm interrupt to do a new measurement */
                pending_interrupt |= INTERRUPT_MASK_RTC;
            }

            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_FXOS8700;
        }

        /* Check if we need to send a message */
        if (send == TRUE)
        {
            /* Build the payload */
            DISCOVERY_build_payload(&payload, MODE_FRIDGE, &data);

            /* Send the message */
            err = RADIO_API_send_message(0xFF, (u8*)&payload, DISCOVERY_PAYLOAD_SIZE, FALSE, NULL);
            /* Parse the error code */
            ERROR_parser(err);

            /* Clear data flags */
            data.button = FALSE;
            data.vibration = FALSE;
            data.magnet = TRUE;

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

/*!******************************************************************
 * \file button_morse.c
 * \brief Sample code to handle morse code with button
 * \author Sens'it Team, modified by Adoption Team
 * \copyright Copyright (c) 2018 Sigfox, All Rights Reserved.
 *******************************************************************/
/******* INCLUDES **************************************************/
#include "../../inc/sensit/sensit_types.h"
#include "../../inc/sensit/sensit_api.h"
#include "../../inc/tools/button_morse.h"
#include "../../inc/tools/serial.h"
#include "../../inc/resources/AT_parser.h"


/******* DEFINES ***************************************************/
#define BUTTON_TIMEOUT          300 /* milliseconds */
#define LETTER_TIMEOUT          3 /* register letter 3 sec after last click */
#define MAX_CLICKS              5 /* 5 clicks max */


/*******************************************************************/
void    LED_control(bool blink, rgb_color_e color)
{
    char i;

    i = 7;
    if (blink)
    {
        while (i--)
        {
            SENSIT_API_set_rgb_led(color);
            SENSIT_API_wait(200);
            SENSIT_API_set_rgb_led(RGB_OFF);
            SENSIT_API_wait(200);
        }
    }
    else
        SENSIT_API_set_rgb_led(color);
}

char    REGISTER_input(button_e *btn, u32 letter, int size)
{
    int     i;

    i = -1;
    while (++i < size)
    {
        if (btn[i] == 1)
            letter = (letter << 1) + 1;
        else
            letter = letter << 1;
    }
    return (letter);
}

int     CHECK_input(u32 letter)
{
    char *morse_bt =
            "**TEMNAIOGKDWRUS**QZYCXBJP*L*FVH09*8***7*****/=61****+**2***3*45";

    if (morse_bt[letter] != '*')
        return (1);
    else
        return (0);
}

void    RESIZE_buffer(u32 *buffer_final, u8 *payload)
{
    int i;
    int j;

    j = 0;
    i = -1;
    while (++i < 3)
    {
        payload[j++] = (buffer_final[i] & 0xFF000000) >> 24;
        payload[j++] = (buffer_final[i] & 0xFF0000) >> 16;
        payload[j++] = (buffer_final[i] & 0xFF00) >> 8;
        payload[j++] = (buffer_final[i] & 0xFF);
    }
}

void    INIT_values(u32 *buffer_final, u8 *payload)
{
    int i;

    i = -1;
    while (++i < 3)
        buffer_final[i] = 0;
    i = -1;
    while (++i < 12)
        payload[i] = 0;
}

void    STORE_input(u32 *buffer_final, u32 letter, int number_char)
{
    int shiftbits;

    if (number_char > 0 && number_char < 6)
    {
        shiftbits = 32 - (number_char * 6);
        buffer_final[0] += letter << shiftbits;
    }
    else if (number_char == 6)
    {
        buffer_final[0] += letter >> 4;
        buffer_final[1] += (letter & 0xF) << 28;
    }
    else if (number_char > 6 && number_char < 11)
    {
        shiftbits = 28 - ((number_char - 6) * 6);
        buffer_final[1] += letter << shiftbits;
    }
    else if (number_char == 11)
    {
        buffer_final[1] += letter >> 2;
        buffer_final[2] += (letter & 0x3) << 30;
    }
    else if (number_char > 11 && number_char < 17)
    {
        shiftbits = 30 - ((number_char - 11) * 6);
        buffer_final[2] += letter << shiftbits;
    }
}

int BUTTON_morse_handler(button_e* buttonArray)
{
    bool buttonState;
    bool lastState;
    u8 i = 0;

    char nbPresses[2]; // to store char value of nb of push

    // register clicks, loop ends on MAX_CLICKS or LETTER_TIMEOUT RTC interrupt
    while(i < MAX_CLICKS)
    {
        SENSIT_API_get_button_state(&buttonState);

        if (!buttonState && lastState) // button is pressed && was previously released
        {
            /* RGB Led ON during count of button presses */
            SENSIT_API_set_rgb_led(RGB_WHITE);

            pending_interrupt &= ~INTERRUPT_MASK_RTC; // clear RTC interrupt
            SENSIT_API_wait(BUTTON_TIMEOUT);
            SENSIT_API_get_button_state(&buttonState);

            if (buttonState) // button is released within BUTTON_TIMEOUT => short click
                buttonArray[i] = BUTTON_ONE_PRESS;
            else
                buttonArray[i] = BUTTON_LONG_PRESS;

            /* RGB Led OFF */
            SENSIT_API_set_rgb_led(RGB_OFF);

            // DEBUG
            //SERIAL_put_data((u8*)AT_itoa(buttonArray[i], nbPresses, 10), 2);
            //SERIAL_put_data((u8*)" - ", 3);

            i++; // Next click

            SENSIT_API_set_rtc_alarm(LETTER_TIMEOUT); // set RTC timeout
            /* Debounce wait */
            SENSIT_API_wait(5);
        }

        // if RTC interrupt (LETTER_TIMEOUT) => exit loop and returns array
        if ((pending_interrupt & INTERRUPT_MASK_RTC) == INTERRUPT_MASK_RTC)
        {
            /* Clear interrupt */
            pending_interrupt &= ~INTERRUPT_MASK_RTC;
            break;
        }

        lastState = buttonState;
    }

    //SERIAL_put_data((u8*)"EXIT LOOP\r\n", 11);
    return (i);
}

/*******************************************************************/
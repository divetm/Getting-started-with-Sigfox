/*!******************************************************************
 * \file button_morse.h
 * \brief Sample code to handle morse code with button
 * \author Sens'it Team, modified by Adoption Team
 * \copyright Copyright (c) 2018 Sigfox, All Rights Reserved.
 *******************************************************************/


/*!******************************************************************
 * \enum button_e
 * \brief Enumaration of button actions.
 *******************************************************************/
typedef enum {
    BUTTON_LONG_PRESS    = 0,
    BUTTON_ONE_PRESS     = 1,
    BUTTON_LAST
} button_e;


/*!************************************************************************
 * \fn void BUTTON_morse_handler(void)
 * \brief Count button presses.
 * \param[out] button_e* buttonArray      Pointer where to write the clicks
 *
 * Both falling edge interrupt & rising edge interrupt must be enabled.
 *
 * \retval array of button_e
 **************************************************************************/
int         BUTTON_morse_handler(button_e* buttonArray);
void        LED_control(bool blink, rgb_color_e color);
char        REGISTER_input(button_e *btn, u32 letter, int size);
int         CHECK_input(u32 letter);
void        STORE_input(u32 *buffer_final, u32 letter, int number_char);
void        RESIZE_buffer(u32 *buffer_final, u8 *payload);
void        INIT_values(u32 *buffer_final, u8 *payload);
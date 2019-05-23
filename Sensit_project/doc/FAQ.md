# F.A.Q.

## I do not receive messages sent by my Sens'it. Why?

1. You may not be using the right Radio Configuration for your current location.

2. The sequence number of your device may have been reset, or too many messages may have been sent without being received.   
   If you have access to your device on the [Sigfox Cloud](https://backend.sigfox.com/), check your device's `Events` page to see if you have a break in message sequence. If you do, you must **disengage the sequence number** with the button located on your device's `Information` page.

3. Check the error code of functions `RADIO_API_send_message` and/or `SIGFOX_API_send_frame` to see if messages are really sent.

## What should I do if my Sens'it stays in bootloader mode after a re-flash?

If your Sens'it does not restart at the end of the re-flash, you must remove the battery and the USB cable to force the restart, or wait for the battery to discharge.


## I have not implemented a way to reset the device in my code. How can I put my Sens'it in bootloader mode again?

You must remove the battery and the USB cable, then press the button and connect the USB cable. Your device will start up in bootloader.

## Why does the main LED of my Sens'it blink alternatively red and white?

A HardFault has occurred.  
It is certainly a segmentation fault. Check pointers and size of arrays in your code.

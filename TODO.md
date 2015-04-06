# Installation #

Need to figure out to reliably install the driver, attach it to devices and make the service start.

I have a couple of tools to help (console.exe, wdffltrinstall.exe, busdog.inf).

At the moment the filter is installed as a lower filter for USB and HID classes via the inf

See http://www.osronline.com/article.cfm?id=446 (KMDF Filter Driver: 30-Minutes -- Installation: Ah...Somewhat Longer) for more information

# busdog usercode reporting #

Currently I have a IOCTL that user code can grab the trace buffer from but the (virtual) machine is severely slowed down when I trace a high volume bulk usb device.

Some ideas to combat this problem:

> - try not to allocate and deallocate trace items all the time (use a ring buffer of trace items and just reallocate if an item needs more space)

> - currently the trace list is synchronized by a WDFWAITLOCK which might be causing troubles from excessive context switching or something maybe I could use a WDFQUEUE (WdfIoQueueCreate etc) apparently they have fast locking mechanisms

## Update ##

Turns out the culprit was just logging was flooding the system (free builds run fine)

# Adjustable debug logging levels #

Probably add another ioctl for this...

## Update ##

Completed

# Manual Queue for user trace buffer requests #

This would be a better design that way the requests would be pending to the user code and we would not have to poll for data.

# Beta Release todo #

  * put version info in code/binaries
  * sign driver for x64 (probably just test signed atm)
  * make sure install/uninstall of driver works nicely
busdog is a filter driver for MS Windows (XP and above) to sniff USB traffic.

If you find busdog useful and would like to donate towards a code signing certificate:
[![](https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=DRDWQPF3J8QRQ)

## Important Note ##

It is a good idea to create a system restore point before mucking about with USB class filter drivers as if you install a filter that windows cannot load (non-signed on x64, unmet dependency etc) then the pnp manager will not be able to load the driver stack for any USB device (all USB mice and keyboards will stop working). A system restore point can come in handy at this stage.

## Installation ##

All you need to do is grab one the execuables and run it, you will then be prompted to install the busdog filter driver. The filter driver sits on top of all USB and HID devices on your system so it is possible to remove it via the setup tab.

Note:
  * If you are running a 64 bit system you will need to either run with testsigning enabled (http://msdn.microsoft.com/en-us/library/dd419910.aspx).
  * The busdog client depends on the .NET 3.5 framework

## Busdog Driver Uninstallation ##

The busdog GUI has an uninstall function on the setup tab and also you can uninstall the driver via the _"Control Panel->Add/Remove Programs"_ interface.


## Screenshots ##
<img src='http://imgur.com/CiANP.png' alt='Busdog device selection screenshot' />
<img src='http://imgur.com/f4p6d.png' alt='Busdog trace view screenshot' />
<img src='http://imgur.com/gQzHu.png' alt='Busdog setup screenshot' />

## Credits ##

Thanks to FAMFAMFAM for the use of the Silk icon set: http://www.famfamfam.com/lab/icons/silk/
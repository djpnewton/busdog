busdog is a filter driver for MS Windows (XP and above) to sniff USB traffic.

If you find busdog useful and would like to donate towards a code signing certificate:
Important Note

## Installation

All you need to do is grab one the execuables and run it, you will then be prompted to install the busdog filter driver. The filter driver sits on top of all USB and HID devices on your system so it is possible to remove it via the setup tab.

Note:

    The busdog client depends on the .NET framework 

## Busdog Driver Uninstallation

The busdog GUI has an uninstall function on the setup tab and also you can uninstall the driver via the "Control Panel->Add/Remove Programs" interface.

## Downloads

[64 bit](http://daniel.net.nz/d/busdog_0.2.2/busdog_x64.exe)
[32 bit](http://daniel.net.nz/d/busdog_0.2.2/busdog_x86.exe)

## Screenshots

![device selection screenshot](images/device_selection.png?raw-true)
![trace view screenshot](images/trace_view.png?raw-true)
![setup screenshot](images/setup.png?raw-true)

## Changelog

### Version 0.2.2 - 2015/05/04

    improvement: Added code certificate so driver can be used on modern (x64) systems

### Version 0.2.1

    improvement: Added support for tracing serial and parallel ports
    improvement: Added "Copy Traces", "Copy Selected Traces" and "Save to File" buttons to trace view 

### Version 0.2.0

    improvement: Added support for tracing usb bus resets and pipe resets
    improvement: Changed trace buffer requests to be blocking (busdog GUI no longer has to poll for data)
    improvement: Added tracing of usb descriptors
    improvement: Added option to automatically start tracing new devices as they arrive
    improvement: Added support for tracing isochronous USB transfers
    improvement: Added detection of mismatched GUI/driver combo
    improvement: Added a maximum trace count option to the trace list tab 

### Version 0.1.2

    bugfix: Added UAC elevation to driver install/uninstall functions
    improvement: added debug symbols to release packages
    improvement: more detailed reporting when driver install/uninstall issues occur 

### Version 0.1.1

    bugfix: fixed driver uninstall failure when the users temporary directory had white-space in the path 

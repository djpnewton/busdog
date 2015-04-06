# Version 0.2.1 #

  * improvement: Added support for tracing serial and parallel ports
  * improvement: Added "Copy Traces", "Copy Selected Traces" and "Save to File" buttons to trace view

# Version 0.2.0 #

  * improvement: Added support for tracing usb bus resets and pipe resets
  * improvement: Changed trace buffer requests to be blocking (busdog GUI no longer has to poll for data)
  * improvement: Added tracing of usb descriptors
  * improvement: Added option to automatically start tracing new devices as they arrive
  * improvement: Added support for tracing isochronous USB transfers
  * improvement: Added detection of mismatched GUI/driver combo
  * improvement: Added a maximum trace count option to the trace list tab

# Version 0.1.2 #

  * bugfix: Added UAC elevation to driver install/uninstall functions
  * improvement: added debug symbols to release packages
  * improvement: more detailed reporting when driver install/uninstall issues occur

# Version 0.1.1 #

  * bugfix: fixed driver uninstall failure when the users temporary directory had white-space in the path
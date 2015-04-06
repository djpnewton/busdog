# Introduction #

The Virtual USB Analyzer (http://vusb-analyzer.sourceforge.net) is a nice looking graphical USB traffic analyzer, I would like to package it up for windows and get busdog to output a USB traffic log that the Virtual USB Analyzer can parse.


# Running vusb-analyzer on windows #

## Prerequisites ##

  * Python 2.6
  * GTK+ for win32 (http://gtk-win.sourceforge.net)
  * pygtk, pygobject, pycairo (http://www.pygtk.org/downloads.html)
  * gnomecanvas (win32 binary: http://ftp.gnome.org/pub/gnome/binaries/win32/libgnomecanvas/, win32 python gnome canvas wrapper...work in progress)
  * ...

## Ideas ##

Maybe I could replace gnomecanvas with a cairo based alternative:

  * Gaphas is a canvas widget for GTK+ implemented with Cairo in pure python
  * goocanvas
  * more at http://cairographics.org/examples/
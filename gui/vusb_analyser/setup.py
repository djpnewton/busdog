from distutils.core import setup
import py2exe

setup(
    name = 'the Virtual USB Analyzer',
    version = 'svn + cairo patch',

    windows = [
                  {
                      'script': 'vusb-analyzer',
                  }
              ],

    options = {
                  'py2exe': {
                      'packages': 'VUsbTools, encodings',
                      'includes': 'cairo, pango, pangocairo, atk, gobject',
                  }
              },
)



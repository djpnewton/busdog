@set VUSB_SRC=vusb_src

:: checkout vusb-analyzer source
if exist vusb_src (
    svn update %VUSB_SRC%
) else (
    svn co https://vusb-analyzer.svn.sourceforge.net/svnroot/vusb-analyzer/trunk %VUSB_SRC%
)

:: copy files
copy setup.py %VUSB_SRC%
copy cairo_port.patch %VUSB_SRC%

:: change dir
cd %VUSB_SRC%

:: apply patch
svn revert *
svn revert VUsbTools\*
del VUsbTools\Canvas.py
patch -p0 -i cairo_port.patch

:: run py2exe
setup.py py2exe

:: change dir back
cd..

:: checkout vusb-analyzer source
if exist vusb_src (
    svn update vusb_src
) else (
    svn co https://vusb-analyzer.svn.sourceforge.net/svnroot/vusb-analyzer/trunk vusb_src
)

:: copy files
copy setup.py vusb_src
copy cairo_port.patch vusb_src

:: change dir
cd vusb_src

:: apply patch
svn revert *
patch -p0 -i cairo_port.patch

:: run py2exe
setup.py py2exe

:: change dir back
cd..

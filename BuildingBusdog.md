# Important Note #

It is a good idea to create a system restore point before mucking about with USB class filter drivers as if you install a filter that windows cannot load (non-signed on x64, unmet dependency etc) then the pnp manager will not be able to load the driver stack for any USB device (all USB mice and keyboards will stop working). A system restore point can come in handy at this stage.

# Building the filter driver #

## Requirements ##

  * Windows Driver Kit 7.1.0 (http://www.microsoft.com/whdc/DevTools)

## Method ##

  * Start a WDK build environment (eg. `Start Menu\Programs\Windows Driver Kits\WDK 7600.16385.1\Build Environments\Windows 7\x86 Checked Build Environment`)
  * Navigate to the busdog filter source directory (eg. `C:\projects\busdog\filter`)
  * Build the filter driver via the "build" command (eg. `build -wg`, build with warnings shown and colored output) which will build the filter driver according to the makefile (`sources`) in that directory.

# Building the GUI #

## Requirements ##

  * Microsoft Visual Studio 2008

## Method ##

  * The solution file `busdog.sln` in the `busdog/gui` directory should build with a standard installation of Visual Studio


# Creating a busdog GUI with embedded driver as a resource #

The released versions of busdog consist of a graphical client with an embedded driver as a resource that can be installed at runtime.

The client and driver are built via a script (`make_package.bat`) that follows this procedure:
  * load the WDK environment
    * build the filter driver
    * sign the driver with the test certificate
    * copy the driver binary and inf to the `gui\driverRes` directory
  * load the Visual Studio environment
    * build the graphical client via the msbuild makefile (`gui\build.xml`)
      * build the driver resource (`gui\driverRes\build.xml`)
      * build the graphical client with driver resources

## Requirements ##

  * Windows Driver Kit 7.1.0 - if not installed to the default location (`C:\WinDDK`) the make\_package.bat script will have to be modified to reflect this
  * Microsoft Visual Studio 2008 - if not installed to the default location (`C:\Program Files" or `C:\Program Files (x86)`) the make\_package.bat script will have to be modified to reflect this

## Method ##

  * Execute the `make_all_packages.bat` script to make both 32 and 64 bit builds (busdog32.exe, busdog64.exe).
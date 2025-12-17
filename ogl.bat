:: use C/C++ exetnsion v 1.28.3 with VS Code!
:: or maybe this stiupid hack will work
set NETFXSDKDir=C:\temp 
:: **************************** VCVARS.BAT FILE ****************************************
:: set %msdev_cmd% to your chosen vcvars bat file
set msdev_cmd="%programfiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

:: ************************** PROJECT FOLDER **********************************************
:: edit  %root% to your actual project root (i.e, parent root of .vscode)
set root=C:/vulkan_ogldev

:: *************************** leave this part alone ********************************************
set path = %path%;%root%
Call %msdev_cmd%
cd /d %root%
code .
pause
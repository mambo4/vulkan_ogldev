:: COMPILE ALL
:: for %%f in (dir *.cpp) DO cl /I src /EHsc /c  %%f

:: COMPILE INDIVIDUAL .cpp FILES    
::cl /I src /EHsc /c  mat4.cpp 
::cl /I src /EHsc /c  quat.cpp 
::cl /I src /EHsc /c  Shader.cpp 
::cl /I src /EHsc /c  transform.cpp
::cl /I src /EHsc /c  vec3.cpp 
::cl /I src /EHsc /c  WinMain.cpp 


:: CLEANUP
::for %%f in (dir *.obj) DO del %%f
del /Q %CD%\src\*.obj
del /Q %CD%\*.obj
del /Q %CD%\build\*.*
del /Q %CD%\bin\*.* 
del /Q %CD%\shaders\*.spv 
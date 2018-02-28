@echo off
cls
set getf=debug
set target=c:\ims5000\bin
echo.

echo Select the files to copy.
echo   1 - Debug files
echo   2 - Release files
choice /c:12 /n

if errorlevel 2 goto SETGET2

:SETGET1
set getf=debug
goto GETTARGET

:SETGET2
set getf=release

:GETTARGET
echo Choose the target directory
echo   1 - C:\IMS5000\BIN
echo   2 - debug
echo   3 - release
choice /c:123 /n
if errorlevel 3 goto SETTARGET3
if errorlevel 2 goto SETTARGET2

:SETTARGET1
set target=c:\ims5000\bin
goto COPYFILES

:SETTARGET2
set target=debug
goto COPYFILES

:SETTARGET3
set target=release
goto COPYFILES

:COPYFILES
if NOT "%target%"=="c:\ims5000\bin" goto COPYFILES2
@echo on
copy %getf%\ims5000.exe %target%
@echo off

if NOT "%getf%"=="debug" goto COPYFILES2
@echo on
copy %getf%\ims5000.pdb %target%

:COPYFILES2
@echo on

copy dde5000\%getf%\dde5000.exe %target%
copy ..\im5000\%getf%\im5000.dll %target%
copy ..\im5000\imtest32\%getf%\imtest32.exe %target%

copy ..\imslib32\%getf%\imslib32.dll %target%
copy ..\jvmath32\%getf%\jvmath32.dll %target%
copy ..\dio24\debug\cbw32.dll %target%
copy ..\dm00132\%getf%\dm00132.dll %target%

copy ..\ij32\ijtest32\%getf%\ijtest32.exe %target%
copy ..\ijdev32\ijsim32\%getf%\ijsim32.dll %target%
copy ..\ijdev32\ijesi24\%getf%\ijesi24.dll %target%
copy ..\ijdev32\ijimaje4\%getf%\ijimaje432.dll %target%
copy ..\ijdev32\ijalphac32\%getf%\ijalphac32.dll %target%
copy ..\ijdev32\ijdomino32\%getf%\ijdomino32.dll %target%

copy ..\xy32\xytest32\%getf%\xytest32.exe %target%
copy ..\xydev32\xysim32\%getf%\xysim32.dll %target%

copy ..\xydev32\rfsim32\%getf%\rfsim32.dll %target%
copy ..\xydev32\rfsx6\%getf%\rfsx632.dll %target%
copy ..\xydev32\rf410\%getf%\rf410.dll %target%

copy ..\xydev32\xyacl32\%getf%\xyacl32.dll %target%
copy ..\xydev32\xy630v\%getf%\xy630v.dll %target%
copy ..\xydev32\xyg1000\%getf%\xyg1000.dll %target%
copy ..\xydev32\xyorion32\%getf%\xyorion32.dll %target%

copy ..\vis32\vistest32\%getf%\vistest32.exe %target%
copy ..\visdev32\vissim32\%getf%\vissim32.dll %target%
copy ..\visdev32\visdmx32\%getf%\visdmx32.dll %target%
copy ..\visdev32\visdvt32\%getf%\visdvt32.dll %target%
copy ..\visdev32\vislaser32\%getf%\vislaser32.dll %target%
copy ..\visdev32\vism20\%getf%\vism20.dll %target%
copy ..\visdev32\vismx600\%getf%\vismx600.dll %target%

@echo off
pause
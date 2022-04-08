@ECHO OFF

set PROJECT_NAME=CustomKeyboard
set EXPORT_APP_DIR=C:\Users\%USERNAME%\Desktop
set MSBUILD_LOC="C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin"

git describe --abbrev=0 --tags >%PROJECT_NAME%
set /p TAG=<%PROJECT_NAME%
git rev-parse --short HEAD >%PROJECT_NAME%
set /p COMMIT_ID=<%PROJECT_NAME%
git symbolic-ref --short HEAD >%PROJECT_NAME%
set /p BRANCH_NAME=<%PROJECT_NAME%
set "basedir=%cd%"
del %PROJECT_NAME%

:INFINITY_LOOP
cd %basedir%
ECHO Select what to do (%PROJECT_NAME% - [%BRANCH_NAME%] %TAG%)
ECHO 1.) Export x86 Debug
ECHO 2.) Export x86 Release
ECHO 3.) Export x64 Debug
ECHO 4.) Export x64 Release
ECHO 5.) Export x64 Static Release
ECHO 6.) Build x86 Debug
ECHO 7.) Build x86 Release
ECHO 8.) Build x86 Static Release
ECHO 9.) Build x64 Debug
ECHO 10.) Build x64 Release
ECHO 11.) Build x64 Static Release

SET /P UserInput=Enter your choice 
SET /A TestVal="%UserInput%"*1

IF %TestVal%==%UserInput% (
  IF %TestVal% == 1 (
    copy /y Debug\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y Debug\wxbase316ud_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase316ud_vc_custom.dll
    copy /y Debug\wxmsw316ud_aui_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_aui_vc_custom.dll
    copy /y Debug\wxmsw316ud_core_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_core_vc_custom.dll
    copy /y Debug\wxmsw316ud_stc_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_stc_vc_custom.dll
    copy /y Debug\wxmsw316ud_propgrid_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_propgrid_vc_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Debug x86 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 2 (
	copy /y Release\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y Release\wxbase316u_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase316u_vc_custom.dll
    copy /y Release\wxmsw316u_aui_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_aui_vc_custom.dll
    copy /y Release\wxmsw316u_core_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_core_vc_custom.dll
    copy /y Release\wxmsw316u_stc_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_stc_vc_custom.dll
    copy /y Release\wxmsw316u_propgrid_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_propgrid_vc_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Release x86 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 3 (
	copy /y x64\Debug\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y x64\Debug\wxbase316ud_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase316ud_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw316ud_aui_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_aui_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw316ud_core_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_core_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw316ud_stc_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_stc_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw316ud_propgrid_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316ud_propgrid_vc_x64_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Debug x64 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
   ) ELSE IF %TestVal% == 4 (
	copy /y x64\Release\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y x64\Release\wxbase316u_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase316u_vc_x64_custom.dll
    copy /y x64\Release\wxmsw316u_aui_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_aui_vc_x64_custom.dll
    copy /y x64\Release\wxmsw316u_core_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_core_vc_x64_custom.dll
    copy /y x64\Release\wxmsw316u_stc_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_stc_vc_x64_custom.dll
    copy /y x64\Release\wxmsw316u_propgrid_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw316u_propgrid_vc_x64_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Release x64 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
   ) ELSE IF %TestVal% == 5 (
	copy /y x64\Static Release\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Static Release x64 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 6 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Debug;Platform=x86
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 7 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Release;Platform=x86
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 8 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration="Static Release";Platform=x86
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 9 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Debug;Platform=x64
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 10 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Release;Platform=x64
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 11 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration="Static Release";Platform=x64
	goto INFINITY_LOOP
  ) ELSE (
    ECHO Exiting...
    goto EOF;
  )
)

:EOF
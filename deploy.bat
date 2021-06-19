@ECHO OFF

set PROJECT_NAME=CustomKeyboard
set EXPORT_APP_DIR=C:\Users\%USERNAME%\Desktop
set MSBUILD_LOC="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\"

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
ECHO 5.) Build x86 Debug
ECHO 6.) Build x86 Release
ECHO 7.) Build x64 Debug
ECHO 8.) Build x64 Release

SET /P UserInput=Enter your choice 
SET /A TestVal="%UserInput%"*1

IF %TestVal%==%UserInput% (
  IF %TestVal% == 1 (
    copy /y Debug\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y Debug\wxbase315ud_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase315ud_vc_custom.dll
    copy /y Debug\wxmsw315ud_aui_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315ud_aui_vc_custom.dll
    copy /y Debug\wxmsw315ud_core_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315ud_core_vc_custom.dll
    copy /y Debug\wxmsw315ud_stc_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315ud_stc_vc_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Debug x86 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 2 (
	copy /y Release\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y Release\wxbase315u_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase315u_vc_custom.dll
    copy /y Release\wxmsw315u_aui_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315u_aui_vc_custom.dll
    copy /y Release\wxmsw315u_core_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315u_core_vc_custom.dll
    copy /y Release\wxmsw315u_stc_vc_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315u_stc_vc_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Release x86 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 3 (
	copy /y x64\Debug\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y x64\Debug\wxbase315ud_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase315ud_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw315ud_aui_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315ud_aui_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw315ud_core_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315ud_core_vc_x64_custom.dll
    copy /y x64\Debug\wxmsw315ud_stc_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315ud_stc_vc_x64_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Debug x64 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
   ) ELSE IF %TestVal% == 4 (
	copy /y x64\Release\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\%PROJECT_NAME%.exe
    copy /y x64\Release\wxbase315u_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxbase315u_vc_x64_custom.dll
    copy /y x64\Release\wxmsw315u_aui_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315u_aui_vc_x64_custom.dll
    copy /y x64\Release\wxmsw315u_core_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315u_core_vc_x64_custom.dll
    copy /y x64\Release\wxmsw315u_stc_vc_x64_custom.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\wxmsw315u_stc_vc_x64_custom.dll
    copy /y Graphs\template.html %EXPORT_APP_DIR%\%PROJECT_NAME%\Graphs\template.html

	ECHO Release x64 successfully deployed to %EXPORT_APP_DIR%\%PROJECT_NAME%
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 5 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Debug;Platform=x86
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 6 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Release;Platform=x86
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 7 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Debug;Platform=x64
	goto INFINITY_LOOP
  ) ELSE IF %TestVal% == 8 (
	CD /D %MSBUILD_LOC%
	msbuild.exe %basedir%/CustomKeyboard.sln /p:configuration=Release;Platform=x64
	goto INFINITY_LOOP
  ) ELSE (
    ECHO Exiting...
    goto EOF;
  )
)

:EOF
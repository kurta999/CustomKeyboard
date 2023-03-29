@ECHO OFF

set PROJECT_NAME=CustomKeyboard
set PROJECT_VERSION=1.4.5
set EXPORT_APP_DIR=C:\Users\%USERNAME%\Desktop

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

copy /y x64\Release\%PROJECT_NAME%.exe %EXPORT_APP_DIR%\%PROJECT_NAME%\CustomKeyboard_v%PROJECT_VERSION%\%PROJECT_NAME%.exe
copy /y x64\Release\%PROJECT_NAME%.pdb %EXPORT_APP_DIR%\%PROJECT_NAME%\CustomKeyboard_v%PROJECT_VERSION%\%PROJECT_NAME%.pdb
del "%EXPORT_APP_DIR%\%PROJECT_NAME%\CustomKeyboard_v%PROJECT_VERSION%.7z"
cd %EXPORT_APP_DIR%\%PROJECT_NAME%
7z.exe a CustomKeyboard_v%PROJECT_VERSION%.7z CustomKeyboard_v%PROJECT_VERSION%\
explorer.exe /select,CustomKeyboard_v%PROJECT_VERSION%.7z
#cd %basedir%

:EOF
@ECHO OFF

set mypath=%cd%
set PROJECT_NAME=CustomKeyboard
set EXPORT_APP_DIR=C:\Users\%USERNAME%\Desktop
set WX_PATH="C:\wxWidgets-3.2.4"
set WX_VERSION=324

set list="wxbase%WX_VERSION%ud_vc_x64_custom" "wxmsw%WX_VERSION%ud_core_vc_x64_custom" "wxmsw%WX_VERSION%ud_stc_vc_x64_custom" "wxmsw%WX_VERSION%ud_propgrid_vc_x64_custom" "wxmsw%WX_VERSION%ud_aui_vc_x64_custom"
(for %%a in (%list%) do ( 
   copy /y %WX_PATH%\lib\vc_x64_dll\%%a.dll %mypath%\x64\Debug\%%a.dll
))

set list="wxbase%WX_VERSION%u_vc_x64_custom" "wxmsw%WX_VERSION%u_core_vc_x64_custom" "wxmsw%WX_VERSION%u_stc_vc_x64_custom" "wxmsw%WX_VERSION%u_propgrid_vc_x64_custom" "wxmsw%WX_VERSION%u_aui_vc_x64_custom"
(for %%a in (%list%) do ( 
   copy /y %WX_PATH%\lib\vc_x64_dll\%%a.dll %mypath%\x64\Release\%%a.dll
   copy /y %WX_PATH%\lib\vc_x64_dll\%%a.dll %EXPORT_APP_DIR%\%PROJECT_NAME%\%%a.dll
))

::set list="wxbase%WX_VERSION%ud_vc_custom" "wxmsw%WX_VERSION%ud_core_vc_custom" "wxmsw%WX_VERSION%ud_stc_vc_custom" "wxmsw%WX_VERSION%ud_propgrid_vc_custom" "wxmsw%WX_VERSION%ud_aui_vc_custom"
::(for %%a in (%list%) do ( 
::   copy /y %WX_PATH%\lib\vc_dll\%%a.dll %mypath%\Debug\%%a.dll
::))
::
::set list="wxbase%WX_VERSION%u_vc_custom" "wxmsw%WX_VERSION%u_core_vc_custom" "wxmsw%WX_VERSION%u_stc_vc_custom" "wxmsw%WX_VERSION%u_propgrid_vc_custom" "wxmsw%WX_VERSION%u_aui_vc_custom"
::(for %%a in (%list%) do ( 
::   copy /y %WX_PATH%\lib\vc_dll\%%a.dll %mypath%\Release\%%a.dll
::))

:EOF
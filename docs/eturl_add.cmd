@echo off
set CWD=%~dp0 
set CWD=%CWD:~0,-2%
set CWD=%CWD:\=\\%
set F=%TEMP%\et.reg
set RPATH=HKEY_CURRENT_USER\Software\Classes\et
rem set RPATH=HKEY_CLASSES_ROOT\et
echo REGEDIT4 > %F%
echo [%RPATH%] >> %F%
echo @="URL:ET (Wolfenstein: Enemy Territory)" >> %F%
echo "URL Protocol"="" >> %F%
echo [%RPATH%\shell\open\command] >> %F%
echo @="\"%CWD%\\ete.exe\" +set fs_basepath \"%CWD%\" +set fs_homepath \"%CWD%\" +connect \"%%1\"" >> %F%
regedit -s %F%
del %F%
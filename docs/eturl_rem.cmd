@echo off
set F=%TEMP%\et.reg
echo REGEDIT4 > %F%
rem echo [-HKEY_CLASSES_ROOT\et] >> %F%
echo [-HKEY_CURRENT_USER\Software\Classes\et] >> %F%
regedit -s %F%
del %F%
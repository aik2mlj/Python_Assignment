@echo off
:start
a
tryBigint
2
fc 1.out 2.out
if errorlevel 1 pause
goto start
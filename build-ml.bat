@echo off
@if "%1"=="" (
set CFG="UNIT_TEST Unicode Debug"
) else (
set CFG=%1
)

call "%VS80COMNTOOLS%\vsvars32.bat"
@echo on

c:\python30\python build.py


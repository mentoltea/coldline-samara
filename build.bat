@echo off
build.exe
if errorlevel 1 (
    run.bat
)
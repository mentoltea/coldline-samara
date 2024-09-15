@echo off
build.exe build.json
if errorlevel 1 (
    run.bat
)
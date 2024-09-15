@echo off
build.exe build-static.json
if errorlevel 1 (
    runstat.bat
)
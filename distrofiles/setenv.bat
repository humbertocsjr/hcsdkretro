@echo off
set "NEW_PATH=%~dp0"

set "PATH=%NEW_PATH%;%PATH%"

start cmd /k "echo HC Software Development Kit"
@echo off
rem Installs the plugin for all DAWs (Cubase, Studio One, FL Studio ...). Right-click -> Run as administrator.
setlocal
set "SRC=%~dp0An-Niyamah Voice Enhance.vst3"
if not exist "%SRC%" set "SRC=%~dp0build\VoiceEnhance_artefacts\Release\VST3\An-Niyamah Voice Enhance.vst3"
if not exist "%SRC%" (
  echo Could not find "An-Niyamah Voice Enhance.vst3" next to this file.
  pause
  exit /b 1
)
set "DST=%CommonProgramFiles%\VST3"
if not exist "%DST%" mkdir "%DST%"
xcopy "%SRC%" "%DST%\An-Niyamah Voice Enhance.vst3\" /E /I /Y >nul
if errorlevel 1 (
  echo Copy failed. Please run this file as Administrator.
  pause
  exit /b 1
)
echo Installed to: %DST%\An-Niyamah Voice Enhance.vst3
echo Now rescan plugins in your DAW.
pause

@echo off
rem Builds the VST3 on your own Windows PC.
rem Needs:  Visual Studio 2022 (workload "Desktop development with C++")  +  CMake  +  Git
cmake -B build -A x64 || goto :err
cmake --build build --config Release --parallel || goto :err
echo.
echo Done. The plugin is here:
echo   build\VoiceEnhance_artefacts\Release\VST3\An-Niyamah Voice Enhance.vst3
pause
exit /b 0
:err
echo Build failed.
pause
exit /b 1

An-Niyamah Voice Enhance  -  VST3 plugin  (version 1.0.0)
=========================================================
Voice Enhance (noise reduction) from An-Niyamah Voice Enhancer, as a VST3 plugin for
Cubase, Nuendo, Studio One, FL Studio, Reaper, Ableton Live, Cakewalk ...  (Windows 64-bit)

BANGLA - কীভাবে ইনস্টল করবেন
-----------------------------
1. "An-Niyamah Voice Enhance.vst3" ফোল্ডারটি এই জায়গায় কপি করুন:
       C:\Program Files\Common Files\VST3\
   (অথবা install_vst3.bat এ রাইট-ক্লিক করে "Run as administrator" দিন - ওটাই কপি করে দেবে)
2. আপনার DAW খুলে প্ল্যাগইন রিস্ক্যান (Rescan) করুন।
3. ভয়েস ট্র্যাকের প্রথম ইনসার্টে "An-Niyamah Voice Enhance" বসান।
4. ১ থেকে ১০ প্রিসেট বেছে নিন: ১ সবচেয়ে হালকা, ১০ সর্বোচ্চ নয়েজ রিমুভ।

ENGLISH - install
-----------------
1. Copy the folder "An-Niyamah Voice Enhance.vst3" to  C:\Program Files\Common Files\VST3\
   (or right-click install_vst3.bat -> Run as administrator).
2. Rescan plugins in your DAW.  3. Insert it FIRST on a voice track.  4. Pick preset 1 - 10.

CONTROLS
--------
 Power button     ON / OFF (timing stays aligned)
 Noise Reduction  0 - 100 %   how much noise may be removed
 Enhanced Mix     0 - 100 %   cleaned voice vs. original
 Presets 1 - 10   ready-made Noise Reduction levels (moving a knob switches to "Custom")
 About menu       About This Plugin / User Guide / About Builders / About An-Niyamah Production & Brand

HOW TO BUILD THE .vst3 (for the developer)
------------------------------------------
Option A - no setup on your PC:  upload this whole folder to a GitHub repository.
           GitHub Actions ("Build Windows VST3") builds it; download the artifact "AnNiyamah-VoiceEnhance-VST3-Windows".
Option B - on your PC:  install Visual Studio 2022 (Desktop development with C++), CMake and Git,
           then double-click build_windows.bat.
The plugin is written in C++ with the JUCE framework (downloaded automatically by CMake).

Source layout
  Source/DSP/NoiseSuppressor.h   noise-reduction engine (no dependencies)
  Source/PluginProcessor.*       parameters, presets, audio processing
  Source/PluginEditor.*          interface, knobs, theme
  Source/AboutWindow.h / AboutContent.h   About windows
  tests/                         offline check of the engine (g++ tests/dsp_test.cpp ; python3 tests/run_dsp_check.py)

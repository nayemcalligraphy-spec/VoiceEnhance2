#pragma once
// The 10 noise-reduction presets (index 0 of the preset parameter is "Custom").
struct NRPreset { const char* name; float nr; float mix; };

static const NRPreset kPresets[10] = {
    { "1 - Whisper Clean",  10.0f,  80.0f },
    { "2 - Light",          20.0f,  80.0f },
    { "3 - Soft",           30.0f,  85.0f },
    { "4 - Natural",        40.0f,  85.0f },
    { "5 - Balanced",       50.0f,  90.0f },
    { "6 - Clear",          60.0f,  90.0f },
    { "7 - Strong",         70.0f,  95.0f },
    { "8 - Studio Clean",   80.0f,  95.0f },
    { "9 - Heavy",          90.0f, 100.0f },
    { "10 - Maximum",      100.0f, 100.0f }
};
static constexpr int kDefaultPreset = 5;      // "5 - Balanced"

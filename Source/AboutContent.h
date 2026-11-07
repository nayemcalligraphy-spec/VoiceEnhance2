#pragma once
// Content of the four About windows (same builders / brand information as the desktop app;
// only the plugin information is new).
#include <JuceHeader.h>
#include <vector>

namespace about
{
inline const char* kPluginName = "An-Niyamah Voice Enhance";
inline const char* kVersion    = "1.0.2";
inline const char* kBrand      = "An-Niyamah Production";

struct LinkRow { juce::String label, text, url; };

struct Item
{
    enum Type { H2, H3, Bold, Para, Muted, Bullet, Links } type;
    juce::String text;
    std::vector<LinkRow> rows;
};

inline Item H2 (juce::String t)     { return { Item::H2, std::move (t), {} }; }
inline Item H3 (juce::String t)     { return { Item::H3, std::move (t), {} }; }
inline Item Bold (juce::String t)   { return { Item::Bold, std::move (t), {} }; }
inline Item Para (juce::String t)   { return { Item::Para, std::move (t), {} }; }
inline Item Muted (juce::String t)  { return { Item::Muted, std::move (t), {} }; }
inline Item Bullet (juce::String t) { return { Item::Bullet, std::move (t), {} }; }
inline Item Links (std::vector<LinkRow> r) { return { Item::Links, {}, std::move (r) }; }

inline std::vector<Item> pluginInfo()
{
    return {
        H2 ("About This Plugin"),
        Bold (juce::String (kPluginName) + "   -   Version " + kVersion),
        Para (juce::String (kPluginName) + " is a free VST3 plugin that removes background noise from a voice in real time. "
              "It cleans hiss, fan and air-conditioner noise, room rumble and electrical hum, and keeps the voice clear, "
              "natural and full - without any harsh or watery sound."),
        H3 ("How it works"),
        Bullet ("It listens to your audio all the time and learns the background noise by itself. There is no \"learn noise\" button "
                "and no noise-only sample is needed."),
        Bullet ("The sound is split into many narrow frequency bands. In every band the plugin decides, moment by moment, how much "
                "is voice and how much is noise, and turns the noise down while the voice passes through."),
        Bullet ("Noise Reduction sets how much noise may be removed - from a light clean-up up to the maximum."),
        Bullet ("Enhanced Mix blends the cleaned voice with the original. Lower it if you want a little of the natural room to stay."),
        Bullet ("The ON / OFF button switches the effect without changing the timing of your track."),
        Bullet ("The 10 Noise Reduction presets give you ready-made levels: 1 is the gentlest and 10 removes the maximum amount of noise. "
                "Move any knob and the preset changes to Custom."),
        H3 ("Where it works"),
        Para ("Windows 64-bit, VST3. Use it in Cubase, Studio One, FL Studio, Reaper, Ableton Live, Nuendo, Cakewalk and any other host "
              "that supports VST3. Mono and stereo tracks, 44.1 kHz to 192 kHz."),
        H3 ("Latency"),
        Para ("The plugin reports its small delay (about 21 ms at 48 kHz) to the host, so your DAW keeps everything in time automatically."),
        H3 ("Privacy"),
        Para ("Everything is processed inside your computer. No audio ever leaves it."),
        Muted (juce::String::fromUTF8 ("\xC2\xA9 2026 ") + kBrand + ". All rights reserved.")
    };
}

inline std::vector<Item> userGuide()
{
    return {
        H2 ("User Guide"),
        Muted (juce::String ("How to use ") + kPluginName),
        H3 ("1. Quick start"),
        Bullet ("Put the plugin on your voice track as the FIRST insert (before EQ, compressor and reverb)."),
        Bullet ("Press play. Choose a Noise Reduction preset from 1 to 10 - start with 5 and go up until the noise is gone."),
        Bullet ("Use the lowest level that cleans your recording. Level 10 is for very noisy recordings."),
        H3 ("2. Controls"),
        Bullet ("Power button (top left): turns Voice Enhance ON or OFF."),
        Bullet ("Noise Reduction: how much noise is removed. Drag up / down, use the mouse wheel, or double-click to reset."),
        Bullet ("Enhanced Mix: how much of the cleaned voice is heard. 100 % is fully cleaned."),
        Bullet ("Preset list and the numbered buttons 1-10: ready-made Noise Reduction levels."),
        Bullet ("Reset: returns to the default preset (5 - Balanced)."),
        Bullet ("Noise Removed meter: shows how much the plugin is reducing right now."),
        H3 ("3. Tips"),
        Bullet ("If the voice sounds a little thin at a high level, lower Enhanced Mix to 90 % or choose the next lower preset."),
        Bullet ("For a recording that is already quiet, levels 1 to 4 are usually enough."),
        Bullet ("All settings are saved with your project and can be automated by your DAW.")
    };
}

inline std::vector<Item> builders()
{
    return {
        H2 ("About Builders & Management"),
        Muted ("Developed and managed by the An-Niyamah Production Team."),
        H3 ("1. Nayem Hasan"),
        Bold ("Founder & CEO"),
        Para ("Professional Arabic Calligrapher (specialized and highly proficient in intricate Arabic Calligraphy arts), "
              "Expert Graphic Designer, and Video Editor."),
        Links ({ { "Facebook Profile",  "facebook.com/nayem.hasan.740700", "https://www.facebook.com/nayem.hasan.740700/" },
                 { "Behance",           "behance.net/nayemcalligraphy",    "https://www.behance.net/nayemcalligraphy" },
                 { "Instagram",         "instagram.com/nayem.hasan.740700", "https://www.instagram.com/nayem.hasan.740700" },
                 { "WhatsApp Contact",  "01831287206",                     "https://wa.me/8801831287206" } }),
        H3 ("2. Ferdous Hasan"),
        Bold ("Co-Founder & Creative Director"),
        Para ("Professional Graphic Designer and Pattern Master, overseeing and managing the creative direction and visual "
              "branding of major brands."),
        Links ({ { "Facebook Page",     "facebook.com/FerdousVisionary2",  "https://www.facebook.com/FerdousVisionary2" },
                 { "Behance",           "behance.net/FerdousVisionary",    "https://www.behance.net/FerdousVisionary" },
                 { "Instagram",         "instagram.com/ferdousvisionary",  "https://www.instagram.com/ferdousvisionary" },
                 { "Twitter",           "twitter.com/FerdousVision",       "https://twitter.com/FerdousVision" },
                 { "WhatsApp Contact",  "+880 1878-776076",                "https://wa.me/8801878776076" } })
    };
}

inline std::vector<Item> brand()
{
    return {
        H2 ("About An-Niyamah Production & Brand"),
        H3 ("An-Niyamah Production & Brand Overview"),
        Bold ("Company Profile"),
        Para ("An-Niyamah Production is a dynamic, multi-disciplinary creative platform specializing in advanced software "
              "development, premium digital design, and high-end multimedia production."),
        Bold ("Exclusive Fragrance Line (An-Niyamah Perfume)"),
        Para ("Driven by passion and artistry, the group proudly operates its exclusive luxury perfume brand, An-Niyamah Perfume. "
              "Specializing in long-lasting, premium Extrait de Parfum concentrations crafted from world-class raw ingredients. "
              "The collection ranges from rich woody and royal oriental spices to fresh, modern contemporary fragrances."),
        H3 ("Core Management & Production Team Members"),
        Bullet ("Nayem Hasan - Founder & CEO"),
        Bullet ("Ferdous Hasan - Co-Founder & Creative Director"),
        Bullet ("Ariful Islam - Chief Financial Officer (CFO)"),
        H3 ("Official Links & Business Contacts"),
        Links ({ { "Official Website",   "anniyamahperfumebd.blogspot.com", "https://anniyamahperfumebd.blogspot.com/" },
                 { "Facebook Page",      "facebook.com/anniyamahperfume",   "https://www.facebook.com/anniyamahperfume" },
                 { "WhatsApp Business",  "01917399552",                     "https://wa.me/8801917399552" } })
    };
}
} // namespace about

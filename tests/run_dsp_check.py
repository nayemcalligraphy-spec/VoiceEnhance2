"""Offline check of the C++ denoiser: synthetic 'voice' + noise, all 10 levels."""
import numpy as np, subprocess, sys
sr = 48000
rng = np.random.default_rng(1)
t = np.arange(sr * 12) / sr
# voice-like: harmonic series with moving pitch + syllable envelope, speech only in some segments
f0 = 120 + 25 * np.sin(2 * np.pi * 0.7 * t)
ph = 2 * np.pi * np.cumsum(f0) / sr
v = sum(np.sin(k * ph) / k for k in range(1, 25))
env = np.clip(np.sin(2 * np.pi * 3.1 * t), 0, 1) ** 0.7
speech_mask = ((t % 4) > 1.0) & ((t % 4) < 3.5)       # pauses: 0-1 s, 3.5-4 s of each 4 s
voice = (0.25 * v * env * speech_mask).astype(np.float32)
white = rng.standard_normal(len(t)).astype(np.float32)
pink = np.cumsum(rng.standard_normal(len(t))).astype(np.float32); pink -= np.convolve(pink, np.ones(4800) / 4800, 'same')
hum = 0.01 * np.sin(2 * np.pi * 50 * t) + 0.005 * np.sin(2 * np.pi * 100 * t)
noise = (0.02 * white + 0.01 * pink / (np.std(pink) + 1e-9) + hum).astype(np.float32)   # roughly -34 dBFS
x = (voice + noise).astype(np.float32)
x.tofile('./in.f32')
print(f"input: voice RMS {20*np.log10(np.sqrt(np.mean(voice[speech_mask]**2))):.1f} dBFS, noise RMS {20*np.log10(np.std(noise)):.1f} dBFS")
quiet = (t >= 8.3) & (t < 8.9)          # inside a pause, away from the tails (latency 1024 samples = 21 ms)
talk = (t >= 9.2) & (t < 11.4) & speech_mask
LAT = 1024
def db(a): return 20 * np.log10(np.sqrt(np.mean(a ** 2)) + 1e-12)
from scipy.signal import butter, sosfilt
HP = butter(4, 150, 'hp', fs=sr, output='sos')   # voice band only (ignore sub-150 Hz rumble of the synthetic noise)
print("level | noise in pauses (dB) | removed >150 Hz (dB) | voice level change (dB) | voice-vs-clean corr")
for lvl in range(1, 11):
    nr = lvl * 10
    out = subprocess.run(['./dsp_test', './in.f32', './out.f32', str(sr), '1', str(nr)], capture_output=True, text=True)
    y = np.fromfile('./out.f32', dtype=np.float32)
    y = np.concatenate([y[LAT:], np.zeros(LAT, np.float32)])      # remove latency
    removed = db(sosfilt(HP, x)[quiet]) - db(sosfilt(HP, y)[quiet])
    vchg = db(y[talk]) - db(x[talk])
    corr = np.corrcoef(y[talk], voice[talk])[0, 1]
    print(f" {lvl:2d}   |  {db(y[quiet]):7.1f}            |  {removed:6.1f}      |  {vchg:6.2f}                 | {corr:.3f}")
assert np.isfinite(y).all()

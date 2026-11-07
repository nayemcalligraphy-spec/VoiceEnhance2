// NoiseSuppressor.h  -  An-Niyamah Voice Enhance
// Real-time adaptive spectral noise suppressor (no external dependencies).
//
//   * 75 % overlapped STFT (sqrt-Hann analysis / synthesis)
//   * noise profile is learned automatically (minimum-statistics tracker, no "learn" button)
//   * decision-directed Wiener gain with over-subtraction and a per-level attenuation limit
//   * time + frequency smoothing of the gain (avoids "musical noise")
//   * stereo-linked: both channels use the same gain, so the stereo image never moves
#pragma once
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <vector>

namespace an
{
class NoiseSuppressor
{
public:
    static constexpr int MaxCh = 2;

    void prepare (double sampleRate, int numChannels)
    {
        sr = sampleRate;
        nch = std::max (1, std::min (MaxCh, numChannels));

        N = (sr <= 50000.0) ? 1024 : (sr <= 100000.0 ? 2048 : 4096);
        H = N / 4;
        K = N / 2 + 1;

        win.resize ((size_t) N);
        for (int i = 0; i < N; ++i)
            win[(size_t) i] = std::sqrt (0.5f * (1.0f - std::cos (2.0f * 3.14159265358979f * (float) i / (float) N)));

        tw.resize ((size_t) N / 2);
        for (int i = 0; i < N / 2; ++i)
        {
            const float a = -2.0f * 3.14159265358979f * (float) i / (float) N;
            tw[(size_t) i] = { std::cos (a), std::sin (a) };
        }
        rev.resize ((size_t) N);
        int bits = 0;
        while ((1 << bits) < N) ++bits;
        for (int i = 0; i < N; ++i)
        {
            int r = 0;
            for (int b = 0; b < bits; ++b)
                if (i & (1 << b)) r |= 1 << (bits - 1 - b);
            rev[(size_t) i] = r;
        }

        subLen = std::max (1, (int) std::lround (0.25 * sr / (double) H));
        for (int c = 0; c < MaxCh; ++c)
        {
            inBuf[c].assign ((size_t) N, 0.0f);
            outAcc[c].assign ((size_t) N, 0.0f);
            outQ[c].assign ((size_t) H, 0.0f);
            dry[c].assign ((size_t) N, 0.0f);
            spec[c].assign ((size_t) N, { 0.0f, 0.0f });
        }
        m2.assign ((size_t) K, 0.0f);
        ps.assign ((size_t) K, 0.0f);
        actMin.assign ((size_t) K, 0.0f);
        subMin.assign ((size_t) (U * K), 0.0f);
        noise.assign ((size_t) K, 1e-9f);
        gs.assign ((size_t) K, 1.0f);
        gf.assign ((size_t) K, 1.0f);
        prevSig2.assign ((size_t) K, 0.0f);
        reset();
        setAmount (amount);
    }

    void reset()
    {
        for (int c = 0; c < MaxCh; ++c)
        {
            std::fill (inBuf[c].begin(), inBuf[c].end(), 0.0f);
            std::fill (outAcc[c].begin(), outAcc[c].end(), 0.0f);
            std::fill (outQ[c].begin(), outQ[c].end(), 0.0f);
            std::fill (dry[c].begin(), dry[c].end(), 0.0f);
        }
        std::fill (gs.begin(), gs.end(), 1.0f);
        std::fill (gf.begin(), gf.end(), 1.0f);
        std::fill (prevSig2.begin(), prevSig2.end(), 0.0f);
        fill = 0;
        dryPos = 0;
        subCount = 0;
        subIdx = 0;
        inited = false;
        reductionSmoothed = 0.0f;
    }

    /** amount: 0..100 (%). Sets how much noise may be removed (attenuation limit) and how hard it is hunted. */
    void setAmount (float percent)
    {
        amount = std::min (100.0f, std::max (0.0f, percent));
        // attenuation limit: ~12 dB at 10 %  ...  80 dB at 100 %  (spread evenly across the 10 presets)
        const float limDb = 8.0f + 72.0f * std::pow (amount / 100.0f, 1.25f);
        gMin = std::pow (10.0f, -limDb / 20.0f);
        beta = 1.0f + 1.6f * (amount / 100.0f);       // over-subtraction: stronger levels remove more
    }

    int getLatencySamples() const { return N; }
    float getReductionDb() const { return reductionSmoothed; }   // <= 0

    /** in: input channels. wet: denoised output. dryOut: input delayed by the latency (for mixing). */
    void process (const float* const* in, float* const* wet, float* const* dryOut, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < nch; ++c)
            {
                const float x = in[c][i];
                inBuf[c][(size_t) (N - H + fill)] = x;
                wet[c][i] = outQ[c][(size_t) fill];
                dryOut[c][i] = dry[c][(size_t) dryPos];
                dry[c][(size_t) dryPos] = x;
            }
            if (++dryPos >= N) dryPos = 0;
            if (++fill >= H)
            {
                fill = 0;
                frame();
            }
        }
    }

private:
    using cpx = std::complex<float>;

    void fft (std::vector<cpx>& x, bool inverse) const
    {
        for (int i = 0; i < N; ++i)
        {
            const int j = rev[(size_t) i];
            if (j > i) std::swap (x[(size_t) i], x[(size_t) j]);
        }
        for (int len = 2; len <= N; len <<= 1)
        {
            const int half = len >> 1, step = N / len;
            for (int s = 0; s < N; s += len)
                for (int k = 0; k < half; ++k)
                {
                    cpx w = tw[(size_t) (k * step)];
                    if (inverse) w = std::conj (w);
                    const cpx a = x[(size_t) (s + k)];
                    const cpx b = x[(size_t) (s + k + half)] * w;
                    x[(size_t) (s + k)] = a + b;
                    x[(size_t) (s + k + half)] = a - b;
                }
        }
    }

    void frame()
    {
        for (int c = 0; c < nch; ++c)
        {
            for (int i = 0; i < N; ++i)
                spec[c][(size_t) i] = { inBuf[c][(size_t) i] * win[(size_t) i], 0.0f };
            fft (spec[c], false);
        }

        for (int k = 0; k < K; ++k)
        {
            float s = 0.0f;
            for (int c = 0; c < nch; ++c) s += std::norm (spec[c][(size_t) k]);
            m2[(size_t) k] = s / (float) nch + 1e-14f;
        }

        // ---- noise tracking (minimum statistics)
        if (! inited)
        {
            for (int k = 0; k < K; ++k)
            {
                ps[(size_t) k] = actMin[(size_t) k] = m2[(size_t) k];
                for (int u = 0; u < U; ++u) subMin[(size_t) (u * K + k)] = m2[(size_t) k];
            }
            inited = true;
        }
        const float a = 0.85f;
        for (int k = 0; k < K; ++k)
        {
            ps[(size_t) k] = a * ps[(size_t) k] + (1.0f - a) * m2[(size_t) k];
            actMin[(size_t) k] = std::min (actMin[(size_t) k], ps[(size_t) k]);
        }
        if (++subCount >= subLen)
        {
            subCount = 0;
            subIdx = (subIdx + 1) % U;
            for (int k = 0; k < K; ++k)
            {
                subMin[(size_t) (subIdx * K + k)] = actMin[(size_t) k];
                actMin[(size_t) k] = ps[(size_t) k];
            }
        }
        for (int k = 0; k < K; ++k)
        {
            float mn = actMin[(size_t) k];
            for (int u = 0; u < U; ++u) mn = std::min (mn, subMin[(size_t) (u * K + k)]);
            noise[(size_t) k] = std::max (mn * biasComp, 1e-14f);
        }

        // ---- gain
        const float alpha = 0.92f;
        const float xiMin = 1e-6f;
        for (int k = 0; k < K; ++k)
        {
            const float nz = noise[(size_t) k];
            const float gammaS = std::max (ps[(size_t) k], 0.5f * m2[(size_t) k]) / nz;   // smoothed a-posteriori SNR
            float xi = alpha * prevSig2[(size_t) k] / nz + (1.0f - alpha) * std::max (gammaS - 1.35f, 0.0f);
            xi = std::max (xi, xiMin) / beta;
            float g = xi / (1.0f + xi);
            g = std::max (g, gMin);
            const float cur = gs[(size_t) k];
            gs[(size_t) k] = cur + (g > cur ? 0.8f : 0.45f) * (g - cur);
        }
        for (int k = 0; k < K; ++k)
        {
            const float l = gs[(size_t) std::max (0, k - 1)], r = gs[(size_t) std::min (K - 1, k + 1)];
            gf[(size_t) k] = std::max (gMin, 0.25f * l + 0.5f * gs[(size_t) k] + 0.25f * r);
        }

        float sIn = 0.0f, sOut = 0.0f;
        for (int k = 0; k < K; ++k)
        {
            const float g2 = gf[(size_t) k] * gf[(size_t) k];
            prevSig2[(size_t) k] = g2 * m2[(size_t) k];
            sIn += m2[(size_t) k];
            sOut += g2 * m2[(size_t) k];
        }
        const float red = 10.0f * std::log10 ((sOut + 1e-12f) / (sIn + 1e-12f));
        reductionSmoothed = 0.9f * reductionSmoothed + 0.1f * red;

        // ---- apply, inverse FFT, overlap-add
        const float scale = 0.5f / (float) N;
        for (int c = 0; c < nch; ++c)
        {
            for (int k = 0; k < K; ++k)
            {
                spec[c][(size_t) k] *= gf[(size_t) k];
                if (k > 0 && k < N / 2) spec[c][(size_t) (N - k)] = std::conj (spec[c][(size_t) k]);
            }
            fft (spec[c], true);
            for (int i = 0; i < N; ++i)
                outAcc[c][(size_t) i] += spec[c][(size_t) i].real() * win[(size_t) i] * scale;
            std::memcpy (outQ[c].data(), outAcc[c].data(), sizeof (float) * (size_t) H);
            std::memmove (outAcc[c].data(), outAcc[c].data() + H, sizeof (float) * (size_t) (N - H));
            std::fill (outAcc[c].begin() + (N - H), outAcc[c].end(), 0.0f);
            std::memmove (inBuf[c].data(), inBuf[c].data() + H, sizeof (float) * (size_t) (N - H));
        }
    }

    static constexpr int U = 6;
    static constexpr float biasComp = 1.7f;

    double sr = 48000.0;
    int nch = 1, N = 1024, H = 256, K = 513, fill = 0, dryPos = 0, subLen = 47, subCount = 0, subIdx = 0;
    bool inited = false;
    float amount = 50.0f, gMin = 0.01f, beta = 1.8f, reductionSmoothed = 0.0f;

    std::vector<float> win, m2, ps, actMin, subMin, noise, gs, gf, prevSig2;
    std::vector<cpx> tw;
    std::vector<int> rev;
    std::vector<float> inBuf[MaxCh], outAcc[MaxCh], outQ[MaxCh], dry[MaxCh];
    std::vector<cpx> spec[MaxCh];
};
} // namespace an

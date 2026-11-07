// usage: dsp_test in.f32 out.f32 sampleRate channels amount
#include "../Source/DSP/NoiseSuppressor.h"
#include <cstdio>
#include <cstdlib>
int main (int argc, char** argv)
{
    if (argc < 6) return 1;
    double sr = atof (argv[3]); int ch = atoi (argv[4]); float amt = (float) atof (argv[5]);
    FILE* f = fopen (argv[1], "rb"); std::vector<float> d; float v;
    while (fread (&v, 4, 1, f) == 1) d.push_back (v);
    fclose (f);
    size_t n = d.size() / (size_t) ch;
    an::NoiseSuppressor s; s.prepare (sr, ch); s.setAmount (amt);
    std::vector<std::vector<float>> in (ch, std::vector<float> (n)), wet = in, dry = in;
    for (size_t i = 0; i < n; ++i) for (int c = 0; c < ch; ++c) in[(size_t) c][i] = d[i * (size_t) ch + (size_t) c];
    const float* ip[2] = { in[0].data(), ch > 1 ? in[1].data() : nullptr };
    float* wp[2] = { wet[0].data(), ch > 1 ? wet[1].data() : nullptr };
    float* dp[2] = { dry[0].data(), ch > 1 ? dry[1].data() : nullptr };
    const size_t blk = 512;
    for (size_t p = 0; p < n; p += blk)
    {
        size_t m = std::min (blk, n - p);
        const float* i2[2] = { ip[0] + p, ch > 1 ? ip[1] + p : nullptr };
        float* w2[2] = { wp[0] + p, ch > 1 ? wp[1] + p : nullptr };
        float* d2[2] = { dp[0] + p, ch > 1 ? dp[1] + p : nullptr };
        s.process (i2, w2, d2, (int) m);
    }
    f = fopen (argv[2], "wb");
    for (size_t i = 0; i < n; ++i) for (int c = 0; c < ch; ++c) fwrite (&wet[(size_t) c][i], 4, 1, f);
    fclose (f);
    printf ("latency %d samples, reduction %.1f dB\n", s.getLatencySamples(), s.getReductionDb());
    return 0;
}

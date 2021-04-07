#include "ps2_core.h"
#include "adsr_core.h"
#include "vga_core.h"

class PianoEffects
{
private:
    static const uint32_t numKeysPerhand = { 12u };
    enum class OctaveSelectState {left = 0, right};
    enum class ValidPresetEnvelopes {def = 0, env1, env2};

    DdfsCore* ddfs_p { nullptr };
    AdsrCore* adsr_p { nullptr };
    Ps2Core*   ps2_p { nullptr };
    FrameCore* frame_p { nullptr };
    OsdCore* osd_p { nullptr };

    uint32_t leftOctave  = { 0 };
    uint32_t rightOctave = { 0 };
    bool lhkeys[numKeysPerhand] = { 0 };
    bool rhkeys[numKeysPerhand] = { 0 };
    OctaveSelectState octSelect = { OctaveSelectState::left };

    static const auto PianoColor = 0060;
    static const auto PressedColor = 0600;
    static const auto pianoWidth = 399;
    static const auto pianoHeight = 100;

public:
    PianoEffects(DdfsCore* ddfs_p, AdsrCore* adsr_p, Ps2Core* ps2_p, FrameCore* frame_p, OsdCore* osd_p): 
    ddfs_p(ddfs_p), adsr_p(adsr_p), ps2_p(ps2_p), frame_p(frame_p), osd_p(osd_p) 
    {
        adsr_p->init();
        adsr_p->select_env(0);
    }

    void playKeyboardState();

private:
    void queryKeyboard();
    void playKeys();
    void setOctave(uint32_t octave);
    void setEnvelope(ValidPresetEnvelopes penv);
    void drawPiano(int upperLeftX, int upperLeftY, bool* keyspressed);
    void flushInputPause_ms(unsigned long sleepTimeMs);
    void dispText(const char*, int row, int startColumn);
};
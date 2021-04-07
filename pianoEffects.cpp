#include "chu_init.h"
#include "pianoEffects.h"

void PianoEffects::playKeyboardState()
{
    dispText("  q   w   e   r   a     s   d   f  z   x   c   v", 3, 5);
    dispText("  u   i   o   p   j     k   l   ;  m   ,   .   /", 20, 25);
    queryKeyboard(); 
    osd_p->wr_char(4, 6, static_cast<uint8_t>(leftOctave + '\x30'));
    drawPiano(40,65, this->lhkeys);
    osd_p->wr_char(24, 24, static_cast<uint8_t>(rightOctave + '\x30'));
    drawPiano(200, 340, this->rhkeys);
    playKeys();
}

void PianoEffects::queryKeyboard()
{
    //*****************************************************querykeyboard constant state
    static constexpr auto switchOctaveSelectKey = char{ '\xF0' };
    //*****************************************************querykeyboard current state logic
    char buf = '\x00';
    if(this->ps2_p->get_kb_ch(&buf))
    {
        if(buf == switchOctaveSelectKey)
        {
            if(this->octSelect == OctaveSelectState::left)
                this->octSelect = OctaveSelectState::right;
            else
                this->octSelect = OctaveSelectState::left;
            
        }
        else if(buf >= 0x30 && buf <= 0x39)
            this->setOctave(uint32_t{buf - 0x30});
        else
        {
            switch(buf)
            {
            case 'q': this->lhkeys[0] = true; break; case 'w': this->lhkeys[1] = true; break; case 'e': this->lhkeys[2] = true; break; case 'r': this->lhkeys[3] = true; break;
            case 'a': this->lhkeys[4] = true; break; case 's': this->lhkeys[5] = true; break; case 'd': this->lhkeys[6] = true; break; case 'f': this->lhkeys[7] = true; break;
            case 'z': this->lhkeys[8] = true; break; case 'x': this->lhkeys[9] = true; break; case 'c': this->lhkeys[10] = true; break; case 'v': this->lhkeys[11] = true; break;

            case 'u': this->rhkeys[0] = true; break; case 'i': this->rhkeys[1] = true; break; case 'o': this->rhkeys[2] = true; break; case 'p': this->rhkeys[3] = true; break;
            case 'j': this->rhkeys[4] = true; break; case 'k': this->rhkeys[5] = true; break; case 'l': this->rhkeys[6] = true; break; case ';': this->rhkeys[7] = true; break;
            case 'm': this->rhkeys[8] = true; break; case ',': this->rhkeys[9] = true; break; case '.': this->rhkeys[10] = true; break; case '/': this->rhkeys[11] = true; break;

            case '!': this->setEnvelope(ValidPresetEnvelopes::def); break; 
            case '@': this->setEnvelope(ValidPresetEnvelopes::env1); break; 
            case '#': this->setEnvelope(ValidPresetEnvelopes::env2); break;
            }
        }
    }
}

void PianoEffects::playKeys()
{
    ddfs_p->set_env_source(1);
    
    bool playedAtLeastOneKey = false;

    auto lkey = 0u;
    for(; lkey < numKeysPerhand; lkey++)
    {
        if(this->lhkeys[lkey]) 
        {
            this->adsr_p->play_note((int)lkey,(int)this->leftOctave, 100);
            playedAtLeastOneKey = true;
            flushInputPause_ms(100);
        }
        this->lhkeys[lkey] = false;
    }

    auto rkey = 0u;
    for(; rkey < numKeysPerhand; rkey++)
    {
        if(rhkeys[rkey])
        {
            this->adsr_p->play_note((int)rkey,(int)this->rightOctave, 100);
            playedAtLeastOneKey = true;
            flushInputPause_ms(100);
        }
        this->rhkeys[rkey] = false;
    }
    if(!playedAtLeastOneKey)
        this->adsr_p->abort();
}

void PianoEffects::setOctave(uint32_t octave) 
{
    if(octSelect == OctaveSelectState::left)
        this->leftOctave = octave;
    else 
        this->rightOctave = octave;
}

void PianoEffects::setEnvelope(ValidPresetEnvelopes penv) 
{
    this->adsr_p->select_env((int)penv);
}

void PianoEffects::drawPiano(int upperLeftX, int upperLeftY, bool* keyspressed)
{
    constexpr auto keyWidthInPix = 57;
    frame_p->drawRectangleOutline(upperLeftX, upperLeftY, pianoWidth, pianoHeight, PianoColor);
    
    static constexpr int regKeyIdxs[7] = {0,2,4,5,7,9,11};
    for(auto i = 0; i < 7; i++)
    {

        const auto xl = upperLeftX + (int)(keyWidthInPix * (i + 1));
        frame_p->plot_line(xl, upperLeftY, xl, upperLeftY + pianoHeight, PianoColor);

        if(keyspressed[regKeyIdxs[i]])
            frame_p->drawRectangleOutline(upperLeftX + (i * keyWidthInPix), upperLeftY, (int)keyWidthInPix, pianoHeight, PressedColor);
    }

    static constexpr int blackKeyIdxs[7] = {0,1,3,0,6,8,10};
    for(auto i = 0; i < 7; i++)
    {
        if(i != 0 && i != 3)
        {
            const auto xr = upperLeftX + (int)(keyWidthInPix * i) - 20;

            frame_p->drawRectangle(xr, upperLeftY, 40, 0.5 * pianoHeight, PianoColor);
            
            if(keyspressed[blackKeyIdxs[i]])
                frame_p->drawRectangleOutline(xr, upperLeftY, 40, 0.5 * pianoHeight, PressedColor);
            else
                frame_p->drawRectangleOutline(xr, upperLeftY, 40, 0.5 * pianoHeight, PianoColor);
        }
    }
}

void PianoEffects::flushInputPause_ms(unsigned long sleepTimeMs)
{
    const auto t0 = now_ms();
    char buf;
    while(now_ms() - t0 < sleepTimeMs)
        this->ps2_p->get_kb_ch(&buf);
}


void PianoEffects::dispText(const char* message, int row, int startColumn)//, int txtHexColor, int bgHexColor)
{
	//osd_p->set_color(txtHexColor, bgHexColor);

	int i = 0;
	while (message[i] != '\0')
	{
		osd_p->wr_char((i + startColumn), row, message[i], 0);
		++i;
	}
}
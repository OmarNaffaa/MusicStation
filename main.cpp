#include "chu_init.h"
#include "gpio_cores.h"
#include "spi_core.h"
#include "sseg_core.h"
#include "ps2_core.h"
#include "ddfs_core.h"
#include "adsr_core.h"
#include "vga_core.h"

#include "soundEffects.h"
#include "pianoEffects.h"
/*
	Global Core Instantiations
*/
// *** MMIO *** //
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
DdfsCore ddfs(get_slot_addr(BRIDGE_BASE, S12_DDFS));
AdsrCore adsr(get_slot_addr(BRIDGE_BASE, S13_ADSR), &ddfs);

// *** Video *** //
OsdCore osd(get_sprite_addr(BRIDGE_BASE, V2_OSD));
FrameCore frame(FRAME_BASE);
SpriteCore mouse(get_sprite_addr(BRIDGE_BASE, V1_MOUSE), 1024);
SpriteCore ghost(get_sprite_addr(BRIDGE_BASE, V3_GHOST), 1024);

/*
	Function Prototypes
*/
void dispText(OsdCore *osd_p, char message[], int row, int startColumn, int txtHexColor, int bgHexColor);
void initProgram(SsegCore *sseg_p, SpriteCore *mouse_p, SpriteCore *ghost_p, DdfsCore *ddfs_p);
void ssegOff(SsegCore *sseg_p);
void drawLoadingBar(FrameCore* frame_p, int xPos, int yPos, int len, int color);

int main()
{
	constexpr int bgColor = 0x800;
	constexpr uint32_t txtBgColor = 0x0002;
	uint16_t swVal, swValPrev;
	initProgram(&sseg, &mouse, &ghost, &ddfs);

	/*
		Messages for each page in interface
	*/
	char homepageMsg_L1[] = "Welcome! Please raise one of the switches listed below:";
	char homepageMsg_L2[] = "* SW 0 - Sound Effect System";
	char homepageMsg_L3[] = "* SW 1 - Keyboard Piano";
	char soundEffectMsg[] = "Tap or shake FPGA (visualization and sound occur on tap)";
	char pianoMsg[] = "Digits 0-9 selects octave, F1 toggles the octave select L<=>R";
	char invalidSwRaisedMsg[] = "Congratulations! Enjoy this loading animation for picking an un-set switch.";

	/*
		Instantiate sound effect (drum) object
	*/
	SoundEffects soundEffectInst(&spi, &ddfs, &frame);
	auto calibrateTap = soundEffectInst.tapCalibration(&spi);

	/*
		Instantiate piano object
	*/
	PianoEffects piano(&ddfs, &adsr, &ps2, &frame, &osd);

	while (1)
	{
		swVal = static_cast<uint16_t>(sw.read());

		// "clear screen" logic
		if (swVal != swValPrev)
		{
			osd.clr_screen();
			frame.clr_screen(bgColor);
		}

		// control logic
		if (swVal == 0) // homepage
		{
			dispText(&osd, homepageMsg_L1, 1, 1, 0xFFF, txtBgColor);
			dispText(&osd, homepageMsg_L2, 3, 4, 0xFFF, txtBgColor);
			dispText(&osd, homepageMsg_L3, 5, 4, 0xFFF, txtBgColor);
		}
		else if (swVal == 1) // "drum" (sound effect system)
		{
			dispText(&osd, soundEffectMsg, 1, 1, 0xFFF, txtBgColor);
			soundEffectInst.soundEffectStation(&spi, &ddfs, &frame, calibrateTap);
		}
		else if (swVal == 2) // electronic piano
		{
			dispText(&osd, pianoMsg, 1, 1, 0xFFF, txtBgColor);
			piano.playKeyboardState();
		}
		else // unspecified switch raised
		{
			dispText(&osd, invalidSwRaisedMsg, 12, 2, 0xFFF, txtBgColor);
			drawLoadingBar(&frame, 20, 260, 600, 0x0F0);
		}

		swValPrev = swVal;
	}
}

void dispText(OsdCore *osd_p, char message[], int row, int startColumn, int txtHexColor, int bgHexColor)
{
	osd_p->set_color(txtHexColor, bgHexColor);

	int i = 0;
	while (message[i] != '\0')
	{
		osd_p->wr_char((i + startColumn), row, message[i], 0);
		++i;
	}
}

void initProgram(SsegCore *sseg_p, SpriteCore *mouse_p, SpriteCore *ghost_p, DdfsCore *ddfs_p)
{
	ssegOff(sseg_p);
	ddfs_p->set_env_source(0);
	ddfs_p->set_env(0.0);
	mouse_p->bypass(1);
	ghost_p->bypass(1);
}

void ssegOff(SsegCore *sseg_p)
{
	int i;
	for (i = 0; i < 8; i++)
		sseg.write_1ptn(0xff, i);
	sseg.set_dp(0x00);
}

void drawLoadingBar(FrameCore* frame_p, int xPos, int yPos, int len, int color)
{
	constexpr int bgColor = 0x800;

	frame_p->bypass(0);
	frame_p->clr_screen(bgColor);

	int i;
	for (i = 0; i < len; i++)
	{
		// draw colored bar
		frame_p->wr_pix((xPos+i), yPos, color);
		frame_p->wr_pix((xPos+i), (yPos+1), color);
		frame_p->wr_pix((xPos+i), (yPos+2), color);
		frame_p->wr_pix((xPos+i), (yPos+3), color);
		frame_p->wr_pix((xPos+i), (yPos+4), color);

		sleep_ms(5);
	}

	int j;
	for (j = 0; j < len; j++)
	{
		// draw remaining bg color to cover bar
		frame_p->wr_pix((xPos+j), yPos, bgColor);
		frame_p->wr_pix((xPos+j), (yPos+1), bgColor);
		frame_p->wr_pix((xPos+j), (yPos+2), bgColor);
		frame_p->wr_pix((xPos+j), (yPos+3), bgColor);
		frame_p->wr_pix((xPos+j), (yPos+4), bgColor);

		sleep_ms(5);
	}
}

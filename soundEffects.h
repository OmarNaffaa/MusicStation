#ifndef _SOUNDEFFECT_H_INCLUDED
#define _SOUNDEFFECT_H_INCLUDED

#include "chu_init.h"
#include "ddfs_core.h"
#include "spi_core.h"
#include "vga_core.h"
#include "math.h"

/*
	Sound Effect
	--> Functions to instantiate a "drum" object with several functions
	----> Detect acceleration and process intensity of acceleration
	----> Produce sound with DDFS core based on tap strength
	----> Animate tap / shake strength to visualize the tap
*/
class SoundEffects
{
public:
	/*
		Constructor
	*/
	SoundEffects(SpiCore *spi_p, DdfsCore *ddfs_p, FrameCore *frame_p);
	~SoundEffects();

	/*
		Methods
	*/
	void soundEffectStation(SpiCore *spi_p, DdfsCore *ddfs_p, FrameCore *frame_p, float tapConst);
	void playSoundEffect(DdfsCore *ddfs_p, float pitch);
	void animateTap(FrameCore *frame_p, float tapStrength);
	float tapCalibration(SpiCore *spi_p);
	float determineTapStrength(SpiCore *spi_p, float tapThresh);
	float captureTapping(SpiCore *spi_p);

private:
	float validTapThres = 0.3;
	float swipeSensitivity = 0.25;

	// Core Instances
	SpiCore *_spi;
	DdfsCore *_ddfs;
	FrameCore *_frame;
};

#endif




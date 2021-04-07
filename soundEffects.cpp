#include "soundEffects.h"

/*
	Constructor
*/
SoundEffects::SoundEffects(SpiCore *spi_p, DdfsCore *ddfs_p, FrameCore *frame_p)
{
	_spi = spi_p;
	_ddfs = ddfs_p;
	_frame = frame_p;
}
SoundEffects::~SoundEffects() {} // not used

/*
	Methods
*/
void SoundEffects::soundEffectStation(SpiCore *spi_p, DdfsCore *ddfs_p, FrameCore *frame_p, float tapConst)
{
	float tapVal = captureTapping(spi_p);

	if ((tapVal - tapConst) > validTapThres)
	{
		tapVal = determineTapStrength(spi_p, tapConst);

		animateTap(frame_p, tapVal);
		playSoundEffect(ddfs_p, tapVal);
	}
}

void SoundEffects::playSoundEffect(DdfsCore *ddfs_p, float pitch)
{
	constexpr auto drumVol_ON = 1.0;
	constexpr auto drumVol_OFF = 0.0;
	constexpr auto maxPitch = 500.0;

	auto normalizeCarrier = 60;
	auto carrierFreq = ((pitch * pitch) / normalizeCarrier);

	// filter carrierFreq to limit sound pitch
	if ((carrierFreq - maxPitch) > 1)
	{
		carrierFreq = maxPitch;
	}

	auto carrierDecrement = carrierFreq / 100;

	ddfs_p->set_env_source(0);      // select envelop source
	ddfs_p->set_env(drumVol_OFF);           // set volume

	ddfs_p->set_carrier_freq(carrierFreq);

	int i;
	for (i = 0; i < 100; i++)
	{
		ddfs_p->set_carrier_freq(static_cast<int>(carrierFreq));
		ddfs_p->set_env(drumVol_ON);

		carrierFreq -= carrierDecrement;

		sleep_ms(1);
	}

	ddfs_p->set_carrier_freq(drumVol_OFF);
	ddfs_p->set_env(0.0);
}

void SoundEffects::animateTap(FrameCore *frame_p, float tapStrength)
{
	frame_p->bypass(0);
	frame_p->clr_screen(0x800);

	int r;
	for (r = 0; r < tapStrength; ++r)
	{
		if (r < 40)
			frame_p->drawCircle(320, 240, r, 0070);
		else if (r >= 40 && r < 80)
			frame_p->drawCircle(320, 240, r, 0060);
		else if (r >= 80 && r < 120)
			frame_p->drawCircle(320, 240, r, 0050);
		else if (r >= 120 && r < 160)
			frame_p->drawCircle(320, 240, r, 0040);
		else if (r >= 160 && r < 200)
			frame_p->drawCircle(320, 240, r, 0030);
		else
			frame_p->drawCircle(320, 240, r, 0020);

		sleep_ms(1);
	}
}

float SoundEffects::tapCalibration(SpiCore *spi_p)
{
	float idle[3] = {0.0, 1.0, 2.0};

	int i = 0;
	while (idle[0] != idle[1] && idle[0] != idle[2] && idle[1] != idle[2])
	{
		idle[i] = captureTapping(spi_p);
		sleep_ms(50);
		i = (i + 1) % 3;
	}

	return idle[0];
}

float SoundEffects::determineTapStrength(SpiCore *spi_p, float tapThresh)
{
	auto accumulatedTap = 0.0;
	auto tapStrength = captureTapping(spi_p);

	while ((tapStrength - tapThresh) > validTapThres)
	{
		tapStrength = captureTapping(spi_p);
		accumulatedTap += tapStrength;
	}

	return accumulatedTap;
}

float SoundEffects::captureTapping(SpiCore *spi_p)
{
	const uint8_t RD_CMD = 0x0b;
	const uint8_t DATA_REG = 0x08;
	const float raw_max = 127.0 / 2.0;  // 128 max 8-bit reading for +/-2

	int8_t xRaw, yRaw, zRaw;
	float x, y, z, acceleration;

	spi_p->set_freq(400000);
	spi_p->set_mode(0, 0);

	// read 8-bit y 'g' values once
	spi_p->assert_ss(0);
	spi_p->transfer(RD_CMD);
	spi_p->transfer(DATA_REG);

	xRaw = spi_p->transfer(0x00);
	yRaw = spi_p->transfer(0x00);
	zRaw = spi_p->transfer(0x00);

	spi_p->deassert_ss(0);

	// normalize values
	x = static_cast<float>(xRaw / raw_max);
	y = static_cast<float>(yRaw / raw_max);
	z = static_cast<float>(zRaw / raw_max);

	acceleration = sqrt((x*x) + (y*y) + (z*z));

	return acceleration;
}

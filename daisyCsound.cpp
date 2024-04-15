#include "daisy_seed.h"
#include "daisysp.h"
#include <stdio.h>
#include "daisyCsound.h"
#include "csound.h"


using namespace daisy;
using namespace daisy::seed;


static int OpenMidiInDevice(CSOUND *csound, void **userData, const char *dev);
static int CloseMidiInDevice(CSOUND *csound, void *userData);
static int
ReadMidiData(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes);
static int OpenMidiOutDevice(CSOUND *csound, void **userData, const char *dev);
static int CloseMidiOutDevice(CSOUND *csound, void *userData);
static int WriteMidiData(CSOUND              *csound,
                         void                *userData,
                         const unsigned char *mbuf,
                         int                  nbytes);


DaisySeed      hw;
MidiUsbHandler midi;
int            cnt = 0;
#define SR 48000
const int numAdcChannels = 12;
CSOUND   *csound;
Pin       adcPins[numAdcChannels]
    = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11};
float       adcVals[12]                         = {0};
const char *controlChannelNames[numAdcChannels] = {"AnalogIn0",
                                                   "AnalogIn1",
                                                   "AnalogIn2",
                                                   "AnalogIn3",
                                                   "AnalogIn4",
                                                   "AnalogIn5",
                                                   "AnalogIn6",
                                                   "AnalogIn7",
                                                   "AnalogIn8",
                                                   "AnalogIn9",
                                                   "AnalogIn10",
                                                   "AnalogIn11"};


void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(int i = 0; i < numAdcChannels; i++)
    {
        csoundSetControlChannel(csound, controlChannelNames[i], adcVals[i]);
    }
    MYFLT *spout = csoundGetSpout(csound);
    int    end   = csoundGetKsmps(csound);
    for(size_t i = 0; i < size; i++)
    {
        if(cnt == 0)
            csoundPerformKsmps(csound);
        out[0][i] = spout[cnt] * 0.5f;
        out[1][i] = spout[cnt + 1] * 0.5f;
        //cnt = cnt != end - 1 ? cnt + 1 : 0; // for mono out
        cnt = (cnt + 2) % (end * 2); // for stereo out
    }
}


int main(void)
{
    CSOUND *cs = csoundCreate(NULL);

    AdcChannelConfig adcConfig[numAdcChannels];
    for(int i = 0; i < numAdcChannels; i++)
    {
        adcConfig[i].InitSingle(adcPins[i]);
    }

    if(cs)
    {
        csound = cs;
        csoundSetOption(cs, "-n");

        int ret = csoundCompileCsdText(cs, csdText.c_str());

        if(ret == 0)
        {
            csoundStart(cs);
            hw.Configure();
            hw.Init();
            Logger<LOGGER_INTERNAL> logger;
            logger.StartLog(); // start serial printing

            hw.adc.Init(adcConfig, numAdcChannels);
            hw.adc.Start();

            MidiUsbHandler::Config midi_cfg;
            midi_cfg.transport_config.periph
                = MidiUsbTransport::Config::INTERNAL;
            midi.Init(midi_cfg);

            hw.SetAudioBlockSize(8); // number of samples handled per callback
            hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
            hw.StartAudio(AudioCallback);

            csoundSetHostData(cs, (void *)&hw);
            csoundSetHostImplementedAudioIO(cs, 1, 0);
            csoundSetHostImplementedMIDIIO(cs, 1);
            csoundSetExternalMidiInOpenCallback(cs, OpenMidiInDevice);
            csoundSetExternalMidiReadCallback(cs, ReadMidiData);
            csoundSetExternalMidiInCloseCallback(cs, CloseMidiInDevice);
            csoundSetExternalMidiOutOpenCallback(cs, OpenMidiOutDevice);
            csoundSetExternalMidiWriteCallback(cs, WriteMidiData);
            csoundSetExternalMidiOutCloseCallback(cs, CloseMidiOutDevice);

            while(1)
            {
                logger.PrintLine("cs = %p \n", cs);
                for(int i = 0; i < numAdcChannels; i++)
                {
                    adcVals[i] = hw.adc.GetFloat(i);
                }
                logger.PrintLine("Analog0 = %f \n", adcVals[0]);
                //midi.Listen();
            }
            csoundReset(cs);
        }
        else
        {
            hw.PrintLine("Error: could not compile csd. \n");
        }
    }
    else
    {
        hw.PrintLine("Error: csoundCreate failed.\n");
        return 1;
    }
    return 0;
}


int OpenMidiInDevice(CSOUND *csound, void **userData, const char *dev)
{
    *userData = (void *)&midi;
    return 0;
}

int CloseMidiInDevice(CSOUND *csound, void *userData)
{
    return 0;
}


int ReadMidiData(CSOUND        *csound,
                 void          *userData,
                 unsigned char *mbuf,
                 int            nbytes)
{
    int n = 0;
    if(userData)
    {
        auto  midiHandler = static_cast<MidiUsbHandler *>(userData);
        auto &transport   = midiHandler->GetTransport();

        while(transport.Readable() && n < nbytes)
        {
            uint8_t byte = transport.Rx();
            *mbuf++      = byte;
            n++;
        }
        return n;
    }
    return 0;
}


int OpenMidiOutDevice(CSOUND *csound, void **userData, const char *dev)
{
    *userData = (void *)&midi;
    return 0;
}

int CloseMidiOutDevice(CSOUND *csound, void *userData)
{
    return 0;
}


int WriteMidiData(CSOUND              *csound,
                  void                *userData,
                  const unsigned char *mbuf,
                  int                  nbytes)
{
    if(userData)
    {
        auto midiHandler = static_cast<MidiUsbHandler *>(userData);
        midiHandler->SendMessage((uint8_t *)mbuf, nbytes);
        return nbytes;
    }
    return 0;
}

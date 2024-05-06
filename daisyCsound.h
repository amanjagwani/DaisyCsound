#ifndef DAISY_CSOUND_H
#define DAISY_CSOUND_H

#include <string>

using namespace std;

string csdText = R"csd(
      <CsoundSynthesizer>
      <CsOptions>
      </CsOptions>
      <CsInstruments>

      sr = 48000
      0dbfs = 1
      nchnls = 2

      

      instr 1
      kdigi digiInDaisy 1, 2
      kcf chnget "AnalogIn0"
      icps cpsmidi
      kcf chnget "AnalogIn0"
      aenv madsr .1, .2, .6, .3
      a1 vco2 0.5, icps*0.995
      a2 vco2 0.5, icps*1.005
      afilt butterlp (a1+a2), 70 + (12000*kcf)
      outs afilt*aenv*kdigi, afilt*aenv*kdigi
      endin

      </CsInstruments>
      <CsScore>
      </CsScore>
      </CsoundSynthesizer>
   )csd";


#endif
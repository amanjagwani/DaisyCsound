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
      ksamps = 32
      0dbfs = 1
      nchnls = 2
      instr 1
      kcf chnget "AnalogIn0"
      iamp ampmidi 1
      icps cpsmidi
      aenv madsr .1, .2, .6, .4
      a1 vco2 iamp, icps
      a2 vco2 iamp, icps
      afilt butterlp (a1+a2), 40 + (kcf * 10000)
      outs afilt*aenv, afilt*aenv
      endin
      //schedule 1, 0, 1000, 74


      </CsInstruments>
      <CsScore>
      </CsScore>
      </CsoundSynthesizer>
   )csd";


#endif
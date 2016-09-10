///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef DSDCC_DSTAR_H_
#define DSDCC_DSTAR_H_

#include <string>

namespace DSDcc
{

class DSDDecoder;

class DSDDstar
{
public:
	DSDDstar(DSDDecoder *dsdDecoder);
   ~DSDDstar();

   void init(bool header = false);
   void process();
   void processHD();

   const std::string& getRpt1() const { return m_rpt1; }
   const std::string& getRpt2() const { return m_rpt2; }
   const std::string& getYourSign() const { return m_yourSign; }
   const std::string& getMySign() const { return m_mySign; }

private:
   typedef enum
   {
       DStarVoiceFrame,
       DStarDataFrame,
       DStarSyncFrame
   } DStarFrameTYpe;

   void initVoiceFrame();
   void initDataFrame();

   void processVoice();
   void processData();
   void processSync();

   void dstar_header_decode();
   void reset_header_strings();

   void storeSymbolDV(int bitindex, unsigned char bit, bool lsbFirst = true);

   DSDDecoder *m_dsdDecoder;
   int m_voiceFrameCount;
   DStarFrameTYpe m_frameType;
   int m_symbolIndex;    //!< Current symbol index in non HD sequence
   int m_symbolIndexHD;  //!< Current symbol index in HD sequence

   // DSTAR
   unsigned char slowdata[4];
   unsigned int bitbuffer;
   const int *w, *x;

   // DSTAR-HD
   std::string m_rpt1;
   std::string m_rpt2;
   std::string m_yourSign;
   std::string m_mySign;

   // constants
   static const int dW[72];
   static const int dX[72];
   static const unsigned char m_terminationSequence[48];
};

} // namespace DSDcc

#endif /* DSTAR_H_ */

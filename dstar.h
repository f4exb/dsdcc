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
#include "viterbi3.h"

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

   const std::string& getRpt1() const { return m_header.m_rpt1; }
   const std::string& getRpt2() const { return m_header.m_rpt2; }
   const std::string& getYourSign() const { return m_header.m_yourSign; }
   const std::string& getMySign() const { return m_header.m_mySign; }

private:
   typedef enum
   {
       DStarVoiceFrame,
       DStarDataFrame,
       DStarSyncFrame
   } DStarFrameTYpe;

   struct DStarHeader
   {
       void clear()
       {
           m_rpt1.clear();
           m_rpt2.clear();
           m_yourSign.clear();
           m_mySign.clear();
       }

       void setRpt1(const char *rpt1, bool force = true) {
           if ((m_rpt1.size() == 0) || force) {
               m_rpt1 = std::string(rpt1, 8);
           }
       }

       void setRpt2(const char *rpt2, bool force = true) {
           if ((m_rpt2.size() == 0) || force) {
               m_rpt2 = std::string(rpt2, 8);
           }
       }

       void setYourSign(const char *yourSign, bool force = true) {
           if ((m_yourSign.size() == 0) || force) {
               m_yourSign = std::string(yourSign, 8);
           }
       }

       void setMySign(const char *mySign, const char *mySignInfo, bool force = true) {
           if ((m_mySign.size() == 0) || force)
           {
               m_mySign = std::string(mySign, 8);
               m_mySign += '/';
               m_mySign += std::string(mySignInfo, 4);
           }
       }

       std::string m_rpt1;
       std::string m_rpt2;
       std::string m_yourSign;
       std::string m_mySign;
   };


   typedef enum
   {
       DStarSlowData0,
       DStarSlowData1,
       DStarSlowData2,
       DStarSlowDataGPS,
       DStarSlowData4,
       DStarSlowDataHeader,
       DStarSlowDataFiller,
       DStarSlowDataNone,
   } DStarSlowDataType;

   struct DStarSlowData
   {
       void init()
       {
           counter = 0;
           radioHeaderIndex = 0;
           currentDataType = DStarSlowDataNone;
       }

       int counter;
       char radioHeader[41];
       int radioHeaderIndex;
       DStarSlowDataType currentDataType;
   };

   void initVoiceFrame();
   void initDataFrame();

   void processVoice();
   void processData();
   void processSlowData(bool firstFrame);
   void processSlowDataByte(unsigned char byte);
   void processSlowDataGroup();
   void processSync();

   void dstar_header_decode();
   void reset_header_strings();

   void storeSymbolDV(int bitindex, unsigned char bit, bool lsbFirst = true);

   DSDDecoder *m_dsdDecoder;
   int m_voiceFrameCount;
   DStarFrameTYpe m_frameType;
   int m_symbolIndex;    //!< Current symbol index in non HD sequence
   int m_symbolIndexHD;  //!< Current symbol index in HD sequence
   Viterbi3 m_viterbi;

   // DSTAR
   unsigned char nullBytes[4];
   unsigned char slowdata[4];
   unsigned int slowdataIx;
   const int *w, *x;

   // DSTAR-HD
   DStarHeader m_header;

   DStarSlowData m_slowData;

   // constants
   static const int dW[72];
   static const int dX[72];
   static const unsigned char m_terminationSequence[48];
};

} // namespace DSDcc

#endif /* DSTAR_H_ */

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

#ifndef DPMR_H_
#define DPMR_H_

namespace DSDcc
{

class DSDDecoder;

class DSDdPMR
{
public:
    DSDdPMR(DSDDecoder *dsdDecoder);
    ~DSDdPMR();

    void init();
    void process();

    int getColorCode() const { return m_colourCode; }
    bool hasSync() const { return m_hasSync; }

private:
    typedef enum
    {
       DPMRHeader,         // FS1 sync header frame (sync detected at upper level)
       DPMRPostFrame,      // frame(s) have been processed and we are looking for a FS2 or FS3 sync
       DPMRSuperFrame,     // process superframe
       DPMREnd             // FS3 sync end frame
    } DPMRState;

    typedef enum
    {
        DPMRVoiceSuperframe, // voice
        DPMRData1Superframe, // data wihout FEC
        DPMRData2Superframe, // data with FEC
    } DPMRSuperframeType;

    void processHeader();
    void processSuperFrame(); // process super frame
    void processEvenFrame();  // process 0 or 2 frames of a super frame
    void processOddFrame();   // process 1 or 3 frames of a super frame
    void processEndFrame();
    void processPostFrame();
    void processColourCode();
    void processPayload(int symbolIndex, int dibit);
    void storeSymbolDV(int dibitindex, unsigned char dibit, bool invertDibit = false);

    DSDDecoder *m_dsdDecoder;
    DPMRState   m_state;
    DPMRSuperframeType m_superframeType;
    char m_syncBuffer[13];   //!< buffer for frame sync: 12  dibits + \0
    char m_colourBuffer[13]; //!< buffer for colour code: 12  dibits + \0
    int m_symbolIndex;       //!< current symbol index in non HD sequence
    int m_frameIndex;        //!< count of frames in superframes since header
    int m_colourCode;        //!< calculated colour code
    bool m_hasSync;

    static const int rW[36];
    static const int rX[36];
    static const int rY[36];
    static const int rZ[36];
    const int *w, *x, *y, *z;
};


} // namespace DSDcc

#endif /* DPMR_H_ */

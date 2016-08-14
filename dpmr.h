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
    typedef enum
    {
    	DPMRNoFrame,         // no sync
        DPMRExtSearchFrame,  // no sync - extensive search
    	DPMRHeaderFrame,     // header
		DPMRPayloadFrame,    // payload superframe not yet determined
        DPMRVoiceSuperframe, // voice superframe
        DPMRData1Superframe, // data superframe wihout FEC
        DPMRData2Superframe, // data superframe with FEC
		DPMREndFrame,        // end frame
    } DPMRFrameType;

    DSDdPMR(DSDDecoder *dsdDecoder);
    ~DSDdPMR();

    void init();
    void process();

    int getColorCode() const { return m_colourCode; }
    DPMRFrameType getFrameType() const { return m_frameType; }

private:
    class LFSRGenerator
    {
    public:
        LFSRGenerator();
        ~LFSRGenerator();

        void init();
        unsigned int next();

    private:
        unsigned int m_sr;
    };

    class Hamming_12_8
    {
    public:
        Hamming_12_8();
        ~Hamming_12_8();

        void init();
        bool decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

    private:
        unsigned char m_corr[16];                       //!< single bit error correction by syndrome index
        static const unsigned char m_H[12*4]; //!< Parity check matrix of bits
    };

    typedef enum
    {
       DPMRHeader,         // FS1 sync header frame (sync detected at upper level)
       DPMRPostFrame,      // frame(s) have been processed and we are looking for a FS2 or FS3 sync
       DPMRExtSearch,      // FS2 or FS3 extensive search
       DPMRSuperFrame,     // process superframe
       DPMREnd             // FS3 sync end frame
    } DPMRState;

    void processHeader();
    void processSuperFrame(); // process super frame
    void processEndFrame();
    void processPostFrame();
    void processExtSearch();
    void processColourCode(int symbolIndex, int dibit);
    void processFS2(int symbolIndex, int dibit);
    void processCCH(int symbolIndex, int dibit);
    void processTCH(int symbolIndex, int dibit);
    void processVoiceFrame(int symbolIndex, int dibit);
    void storeSymbolDV(int dibitindex, unsigned char dibit, bool invertDibit = false);
    void initScrambling();
    void initInterleaveIndexes();
    bool checkCRC7(unsigned char *bits, int nbBits);
    bool checkCRC8(unsigned char *bits, int nbBits);

    DSDDecoder *m_dsdDecoder;
    DPMRState   m_state;
    DPMRFrameType m_frameType;
    unsigned char m_syncDoubleBuffer[24]; //!< double buffer for frame sync extensive search
    unsigned char  m_colourBuffer[12];    //!< buffer for colour code: 12  dibits
    int  m_syncCycle;
    int  m_symbolIndex;                   //!< current symbol index in non HD sequence
    int  m_frameIndex;                    //!< count of frames in superframes since header
    int  m_colourCode;                    //!< calculated colour code
    LFSRGenerator m_scramblingGenerator;
    Hamming_12_8  m_hamming;
    unsigned char m_scrambleBits[120];
    unsigned char m_bitBufferRx[120];
    unsigned char m_bitBuffer[80];
    unsigned char m_bitWork[80];
    unsigned int dI72[72];
    unsigned int dI120[120];

    static const int rW[36];
    static const int rX[36];
    static const int rY[36];
    static const int rZ[36];
    static const unsigned char m_fs2[12];
    static const unsigned char m_fs3[12];
    static const unsigned char m_preamble[12];
    const int *w, *x, *y, *z;
};


} // namespace DSDcc

#endif /* DPMR_H_ */

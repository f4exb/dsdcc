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

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "dsd_decoder.h"
#include "dsd_upsample.h"

#ifdef DSD_USE_SERIALDV
#include "dvcontroller.h"
#endif
int exitflag;

class Mixer
{
public:
    Mixer() : m_mix(0), m_mixSize(0), m_mixSizeMax(0) {}
    ~Mixer() {
        if (m_mix != 0) {
            delete[] m_mix;
        }
    }
    void mix(unsigned int size1, unsigned int size2, short *channel1, short *channel2);
    short *getMix(int &mixSize) { mixSize = m_mixSize; return m_mix; }

private:
    short *m_mix;
    unsigned int m_mixSize;
    unsigned int m_mixSizeMax;
};

void Mixer::mix(unsigned int size1, unsigned int size2, short *channel1, short *channel2)
{
    unsigned int m_mixSize = std::max(size1, size2);

    if (m_mixSize > m_mixSizeMax)
    {
        if (m_mix != 0)
        {
            delete[] m_mix;
        }

        m_mix = new short[m_mixSizeMax];
        m_mixSizeMax = m_mixSize;
    }

    for (unsigned int i = 0; i < m_mixSize; i++)
    {
        if (i < size1) {
            m_mix[i] = channel1[i];
        } else {
            m_mix[i] = 0;
        }

        if (i < size2) {
            m_mix[i] = (m_mix[i]/2) + (channel2[i]/2);
        } else {
            m_mix[i] /= 2;
        }
    }
}

static void usage ();
static void sigfun (int sig);

void usage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  dsd [options] Live scanner mode\n");
    fprintf(stderr, "  dsd -h        Show help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Display Options:\n");
    fprintf(stderr, "  -e            Show Frame Info and errorbars (default)\n");
    fprintf(stderr, "  -pe           Show P25 encryption sync bits - not supported\n");
    fprintf(stderr, "  -pl           Show P25 link control bits - not supported\n");
    fprintf(stderr, "  -ps           Show P25 status bits and low speed data - not supported\n");
    fprintf(stderr, "  -pt           Show P25 talkgroup info - not supported\n");
    fprintf(stderr, "  -q            Don't show Frame Info/errorbars\n");
    fprintf(stderr, "  -t            Show symbol timing during sync\n");
    fprintf(stderr, "  -v <num>      Frame information Verbosity\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Input/Output options:\n");
    fprintf(stderr, "  -i <device>   Audio input device (default is /dev/audio, - for piped stdin)\n");
    fprintf(stderr, "  -o <device>   Audio output device (default is /dev/audio, - for stdout)\n");
    fprintf(stderr, "  -g <num>      Audio output gain (default = 0 = auto, disable = -1)\n");
    fprintf(stderr, "  -U <num>      Audio output upsampling\n");
    fprintf(stderr, "                0: no upsampling (8k) default\n");
    fprintf(stderr, "                6: normal upsampling to 48k\n");
    fprintf(stderr, "                7: 7x upsampling to trade audio drops against bad audio quality\n");
    fprintf(stderr, "  -n            Do not send synthesized speech to audio output device\n");
    fprintf(stderr, "  -L <filename> Log messages to file with file name <filename>. Default is stderr\n");
    fprintf(stderr, "                If file name is invalid messages will go to stderr\n");
    fprintf(stderr, "  -M <filename> Log formatted messages to file with file name <filename>. Default is none\n");
    fprintf(stderr, "                Formatted messages contain traffic information such as IDs and callsigns\n");
    fprintf(stderr, "                Fields and their column position depend on the frame type\n");
    fprintf(stderr, "  -m <float>    Formatted messages refresh rate in seconds. Default is 0.1\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Scanner control options:\n");
    fprintf(stderr,
            "  -R <num>      Resume scan after <num> TDULC frames or any PDU or TSDU\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Decoder options:\n");
    fprintf(stderr, "  -d <num>      Set data rate:\n");
    fprintf(stderr, "     0          2400 bauds\n");
    fprintf(stderr, "     1          4800 bauds (default)\n");
    fprintf(stderr, "     2          9800 bauds\n");
    fprintf(stderr, "  -fa           Auto-detect frame type (default)\n");
    fprintf(stderr, "  -fr           Decode only DMR/MOTOTRBO\n");
    fprintf(stderr, "  -fd           Decode only D-STAR\n");
    fprintf(stderr, "  -fm           Decode only DPMR Tier 1 or 2 (6.25 kHz)\n");
    fprintf(stderr, "  -fy           Decode only YSF\n");
    fprintf(stderr, "  -fi           Decode only NXDN48 (6.25 kHz) / IDAS* - detection only\n");
    fprintf(stderr, "  -fn           Decode only NXDN96 (12.5 kHz)  - detection only\n");
    fprintf(stderr, "  -f1           Decode only P25 Phase 1 - not supported\n");
    fprintf(stderr, "  -fp           Decode only ProVoice - not supported\n");
    fprintf(stderr, "  -fx           Decode only X2-TDMA - not supported\n");
    fprintf(stderr, "  -T <num>      TDMA slots processed:\n");
    fprintf(stderr, "     0          none\n");
    fprintf(stderr, "     1          slot #1 (default) use this one for FDMA\n");
    fprintf(stderr, "     2          slot #2\n");
    fprintf(stderr, "     3          slots #1+2 mixed\n");
    fprintf(stderr, "  -l            Disable matched filter\n");
    fprintf(stderr, "  -pu           Unmute Encrypted P25 - not supported\n");
    fprintf(stderr, "  -u <num>      Unvoiced speech quality (default=3)\n");
#ifdef DSD_USE_SERIALDV
    fprintf(stderr, "  -D <device>   Use DVSI AMBE3000 based device for AMBE decoding (e.g. ThumbDV)\n");
    fprintf(stderr, "                You must have compiled with serialDV support (see Readme.md)\n");
    fprintf(stderr, "                Device name is the corresponding TTY USB device e.g /dev/ttyUSB0\n");
#endif
    fprintf(stderr, "  -H            Use high-pass filter on audio when using mbelib\n");
    fprintf(stderr, "  -P <float>    Own latitude in decimal degrees. Latitude is positive to the North. Default 0\n");
    fprintf(stderr, "  -Q <float>    Own longitue in decimal degrees. Longitude is positive to the East. Default 0\n");
    fprintf(stderr, "                This is useful when status messages (see -M option) contain geographical data\n");
    fprintf(stderr, "                Practically this is only applicable to D-Star\n");
    fprintf(stderr, "  -x            Disable symbol PLL lock\n");
    fprintf(stderr, "  -k <num>      Number of Basic Privacy key for DMR [1..255]\n");
    fprintf(stderr, "\n");
    exit(0);
}

void sigfun(int sig __attribute__((unused)))
{
    exitflag = 1;
    signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
    int c;
    extern char *optarg;
    extern int optind __attribute__((unused));
    extern int optopt __attribute__((unused));
    extern int opterr;
    DSDcc::DSDDecoder dsdDecoder;
    DSDcc::DSDUpsampler upsamplingEngine;
    char in_file[1023];
    int  in_file_fd = -1;
    char out_file[1023];
    int  out_file_fd = -1;
    char log_file[1023];
    log_file[0] = '\0';
    char formattext_file[1023];
    formattext_file[0] = '\0';
    FILE *formattext_fp = 0;
    float formattext_refresh = 0.1f;
    char formattext[128];
#ifdef DSD_USE_SERIALDV
    char serialDevice[16];
    int dvGain_dB = 0;
    std::string dvSerialDevice;
#endif
    int slots = 1;
    Mixer mixer;
    float lat = 0.0f;
    float lon = 0.0f;

    fprintf(stderr, "Digital Speech Decoder DSDcc\n");

    exitflag = 0;
    signal(SIGINT, sigfun);

    while ((c = getopt(argc, argv,
            "hHep:qtv:i:o:g:nR:f:u:U:lL:D:d:T:M:m:P:Q:xk:")) != -1)
    {
        opterr = 0;
        switch (c)
        {
        case 'h':
            usage();
            exit(0);
        case 'H':
            dsdDecoder.useHPMbelib(true);
            break;
        case 'e':
            dsdDecoder.showErrorBars();
            break;
        case 'p':
            if (optarg[0] == 'e')
            {
                dsdDecoder.setP25DisplayOptions(DSDcc::DSDDecoder::DSDShowP25EncryptionSyncBits, true);
            }
            else if (optarg[0] == 'l')
            {
                dsdDecoder.setP25DisplayOptions(DSDcc::DSDDecoder::DSDShowP25LinkControlBits, true);
            }
            else if (optarg[0] == 's')
            {
                dsdDecoder.setP25DisplayOptions(DSDcc::DSDDecoder::DSDShowP25EncryptionSyncBits, true);
            }
            else if (optarg[0] == 't')
            {
                dsdDecoder.setP25DisplayOptions(DSDcc::DSDDecoder::DSDShowP25TalkGroupInfo, true);
            }
            else if (optarg[0] == 'u')
            {
                dsdDecoder.muteEncryptedP25(false);
            }
            break;
        case 'q':
            dsdDecoder.setQuiet();
            break;
        case 't':
            dsdDecoder.showSymbolTiming();
            break;
        case 'v':
            int verbosity;
            sscanf(optarg, "%d", &verbosity);
            dsdDecoder.setLogVerbosity(verbosity);
            break;
        case 'L':
            strncpy(log_file, (const char *) optarg, 1023);
            log_file[1022] = '\0';
            break;
        case 'M':
            strncpy(formattext_file, (const char *) optarg, 1023);
            formattext_file[1022] = '\0';
            break;
        case 'm':
            float rate;
            sscanf(optarg, "%f", &rate);
            if (rate > 0.1f) {
                formattext_refresh = rate;
            }
            break;
        case 'i':
            strncpy(in_file, (const char *) optarg, 1023);
            in_file[1022] = '\0';
            break;
        case 'o':
            strncpy(out_file, (const char *) optarg, 1023);
            out_file[1022] = '\0';
            break;
#ifdef DSD_USE_SERIALDV
        case 'D':
            strncpy(serialDevice, (const char *) optarg, 16);
            serialDevice[15] = '\0';
            dvSerialDevice = serialDevice;
            break;
#endif
        case 'g':
            float gain;
            sscanf(optarg, "%f", &gain);
            dsdDecoder.setAudioGain(gain);
#ifdef DSD_USE_SERIALDV
            if (gain > 0) {
                dvGain_dB = (int) (10.0f * log10f(gain));
            }
#endif
            break;
        case 'n':
            dsdDecoder.enableAudioOut(false);
            break;
        case 'R':
            int resume;
            sscanf(optarg, "%d", &resume);
            dsdDecoder.enableScanResumeAfterTDULCFrames(resume);
            break;
        case 'd':
            int dataRateIndex;
            sscanf(optarg, "%d", &dataRateIndex);
            if ((dataRateIndex >= 0) && (dataRateIndex <= 2))
            {
                dsdDecoder.setDataRate((DSDcc::DSDDecoder::DSDRate) dataRateIndex);
            }
            break;
        case 'T':
            int tmpSlots;
            sscanf(optarg, "%d", &tmpSlots);
            if ((tmpSlots >= 0) && (tmpSlots <= 3))
            {
                slots = tmpSlots;
            }
            break;
        case 'f':
            dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeNone, true);
            if (optarg[0] == 'a') // auto detect
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeAuto, true);
            }
            else if (optarg[0] == 'r') // DMR/MOTOTRBO
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeDMR, true);
            }
            else if (optarg[0] == 'd') // D-Star
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeDStar, true);
            }
            else if (optarg[0] == 'x') // X2-TDMA
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeX2TDMA, true);
            }
            else if (optarg[0] == 'p') // ProVoice
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeProVoice, true);
            }
            else if (optarg[0] == '0') // P25 Phase 1
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeP25P1, true);
            }
            else if (optarg[0] == 'i') // NXDN48 IDAS
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeNXDN48, true);
            }
            else if (optarg[0] == 'n') // NXDN96
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeNXDN96, true);
            }
            else if (optarg[0] == 'm') // DPMR Tier 1 or 2
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeDPMR, true);
            }
            else if (optarg[0] == 'y') // YSF
            {
                dsdDecoder.setDecodeMode(DSDcc::DSDDecoder::DSDDecodeYSF, true);
            }
            break;
        case 'u':
            int uvquality;
            sscanf(optarg, "%i", &uvquality);
            dsdDecoder.setUvQuality(uvquality);
            break;
        case 'U':
            int upsampling;
            sscanf(optarg, "%d", &upsampling);
            dsdDecoder.setUpsampling(upsampling);
            break;
        case 'l':
            dsdDecoder.enableCosineFiltering(false);
            break;
        case 'P':
            sscanf(optarg, "%f", &lat);
            break;
        case 'Q':
            sscanf(optarg, "%f", &lon);
            break;
        case 'x':
            dsdDecoder.setSymbolPLLLock(false);
            break;
        case 'k':
            int key_number;
            sscanf(optarg, "%u", &key_number);
            dsdDecoder.setDMRBasicPrivacyKey(key_number);
            break;
        default:
            usage();
            exit(0);
        }
    }

    dsdDecoder.setMyPoint(lat, lon);

    if (strlen(log_file) > 0) {
        dsdDecoder.setLogFile(log_file);
    }

    if (strncmp(in_file, (const char *) "-", 1) == 0)
    {
        in_file_fd = STDIN_FILENO;
    }
    else
    {
        in_file_fd = open(in_file, O_RDONLY);
    }

    if (in_file_fd > -1)
    {
        fprintf(stderr, "Opened %s for input.\n", in_file);
    }
    else
    {
        fprintf(stderr, "Cannot open %s for input. Aborting\n", in_file);
        return 0;
    }

    if (strncmp(out_file, (const char *) "-", 1) == 0)
    {
        out_file_fd = STDOUT_FILENO;
    }
    else
    {
        out_file_fd = open(out_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }

    if (out_file_fd > -1)
    {
        fprintf(stderr, "Opened %s for output.\n", in_file);
    }
    else
    {
        if (in_file_fd != STDIN_FILENO) {
            close(in_file_fd);
        }

        fprintf(stderr, "Cannot open %s for output. Aborting\n", out_file);
        return 0;
    }

#ifdef DSD_USE_SERIALDV
    SerialDV::DVController dvController;
    short dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE * 8];

    if (!dvSerialDevice.empty())
    {
        if (dvController.open(dvSerialDevice))
        {
            fprintf(stderr, "SerialDV up. Disable mbelib support\n");
            dsdDecoder.enableMbelib(false); // de-activate mbelib decoding
        }
    }
#endif

    int formattext_nsamples;

    if (formattext_file[0] == 0)
    {
        formattext_nsamples = 0;
    }
    else
    {
        formattext_nsamples = 48000.0f * formattext_refresh;
        formattext_fp = fopen(formattext_file, "w");

        if (!formattext_fp)
        {
            formattext_nsamples = 0;
        }
    }

    int formattext_sample_count = 0;

    while (exitflag == 0)
    {
        short sample;
        int nbAudioSamples1 = 0, nbAudioSamples2 = 0;
        short *audioSamples1, *audioSamples2;

        int result = read(in_file_fd, (void *) &sample, sizeof(short));

        if (result == 0)
        {
            fprintf(stderr, "No more input\n");
            break;
        }

        dsdDecoder.run(sample);

#ifdef DSD_USE_SERIALDV
        if (dvController.isOpen())
        {
            if (dsdDecoder.mbeDVReady1())
            {
                dvController.decode(dvAudioSamples, (const unsigned char *) dsdDecoder.getMbeDVFrame1(), (SerialDV::DVRate) dsdDecoder.getMbeRate(), dvGain_dB);

                if (dsdDecoder.upsampling())
                {
                    upsamplingEngine.upsample(dsdDecoder.upsampling(), dvAudioSamples, &dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE], SerialDV::MBE_AUDIO_BLOCK_SIZE);
                    result = write(out_file_fd, (const void *) &dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE], SerialDV::MBE_AUDIO_BLOCK_BYTES * dsdDecoder.upsampling());
                }
                else
                {
                    result = write(out_file_fd, (const void *) dvAudioSamples, SerialDV::MBE_AUDIO_BLOCK_BYTES); // TODO: upsampling
                }

                dsdDecoder.resetMbeDV1();
            }

            if (dsdDecoder.mbeDVReady2())
            {
                dvController.decode(dvAudioSamples, (const unsigned char *) dsdDecoder.getMbeDVFrame2(), (SerialDV::DVRate) dsdDecoder.getMbeRate(), dvGain_dB);

                if (dsdDecoder.upsampling())
                {
                    upsamplingEngine.upsample(dsdDecoder.upsampling(), dvAudioSamples, &dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE], SerialDV::MBE_AUDIO_BLOCK_SIZE);
                    result = write(out_file_fd, (const void *) &dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE], SerialDV::MBE_AUDIO_BLOCK_BYTES * dsdDecoder.upsampling());
                }
                else
                {
                    result = write(out_file_fd, (const void *) dvAudioSamples, SerialDV::MBE_AUDIO_BLOCK_BYTES); // TODO: upsampling
                }

                dsdDecoder.resetMbeDV2();
            }
        }
        else
#endif
        {
            if (slots & 1)
            {
                audioSamples1 = dsdDecoder.getAudio1(nbAudioSamples1);
            }

            if (slots & 2)
            {
                audioSamples2 = dsdDecoder.getAudio2(nbAudioSamples2);
            }

            if ((nbAudioSamples1 > 0) && (nbAudioSamples2 == 0))
            {
                result = write(out_file_fd, (const void *) audioSamples1, sizeof(short) * nbAudioSamples1);

                if (result < 0)
                {
                    fprintf(stderr, "Error writing to output\n");
                }
                else if ((unsigned int) result != sizeof(short) * nbAudioSamples1)
                {
                    fprintf(stderr, "Written %d out of %d audio samples\n", result/2, nbAudioSamples1);
                }

                dsdDecoder.resetAudio1();
            }

            if ((nbAudioSamples2 > 0) && (nbAudioSamples1 == 0))
            {
                result = write(out_file_fd, (const void *) audioSamples2, sizeof(short) * nbAudioSamples2);

                if (result < 0)
                {
                    fprintf(stderr, "Error writing to output\n");
                }
                else if ((unsigned int) result != sizeof(short) * nbAudioSamples2)
                {
                    fprintf(stderr, "Written %d out of %d audio samples\n", result/2, nbAudioSamples2);
                }

                dsdDecoder.resetAudio2();
            }

            if ((nbAudioSamples1 > 0) && (nbAudioSamples2 > 0))
            {
                short *mix;
                int mixSize;

                mixer.mix(nbAudioSamples1, nbAudioSamples2, audioSamples1, audioSamples2);
                mix = mixer.getMix(mixSize);

                result = write(out_file_fd, (const void *) mix, sizeof(short) * mixSize);

                if (result < 0)
                {
                    fprintf(stderr, "Error writing to output\n");
                }
                else if ((unsigned int) result != sizeof(short) * mixSize)
                {
                    fprintf(stderr, "Written %d out of %d audio samples\n", result/2, mixSize);
                }

                dsdDecoder.resetAudio1();
                dsdDecoder.resetAudio2();
            }
        }

        if (formattext_nsamples > 0)
        {
            if (formattext_sample_count < formattext_nsamples)
            {
                formattext_sample_count++;
            }
            else
            {
                dsdDecoder.formatStatusText(formattext);
                fputs(formattext, formattext_fp);
                putc('\n', formattext_fp);
                formattext_sample_count = 0;
            }
        }
    }

    if (formattext_fp)
    {
        fclose(formattext_fp);
    }

    fprintf(stderr, "End of process\n");

#ifdef DSD_USE_SERIALDV
    if (dvController.isOpen()) {
        dvController.close();
    }
#endif

    if ((out_file_fd > -1) && (out_file_fd != STDOUT_FILENO)) {
        close(out_file_fd);
    }

    if ((in_file_fd > -1) && (in_file_fd != STDIN_FILENO)) {
        close(in_file_fd);
    }

    return (0);
}

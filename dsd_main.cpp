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
    fprintf(stderr, "  -pe           Show P25 encryption sync bits\n");
    fprintf(stderr, "  -pl           Show P25 link control bits\n");
    fprintf(stderr, "  -ps           Show P25 status bits and low speed data\n");
    fprintf(stderr, "  -pt           Show P25 talkgroup info\n");
    fprintf(stderr, "  -q            Don't show Frame Info/errorbars\n");
    fprintf(stderr, "  -s            Datascope (disables other display options)\n");
    fprintf(stderr, "  -t            Show symbol timing during sync\n");
    fprintf(stderr, "  -v <num>      Frame information Verbosity\n");
    fprintf(stderr, "  -z <num>      Frame rate for datascope\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Input/Output options:\n");
    fprintf(stderr,
            "  -i <device>   Audio input device (default is /dev/audio, - for piped stdin)\n");
    fprintf(stderr, "  -o <device>   Audio output device (default is /dev/audio, - for stdout)\n");
    fprintf(stderr, "  -g <num>      Audio output gain (default = 0 = auto, disable = -1)\n");
    fprintf(stderr, "  -U <num>      Audio output upsampling\n");
    fprintf(stderr, "                0: no upsampling (8k) default\n");
    fprintf(stderr, "                6: normal upsampling to 48k\n");
    fprintf(stderr, "                7: 7x upsampling to trade audio drops against bad audio quality\n");
    fprintf(stderr, "  -n            Do not send synthesized speech to audio output device\n");
    fprintf(stderr, "  -L <filename> Log messages to file with file name <filename>. Default is stderr\n");
    fprintf(stderr, "                If file name is invalid messages will go to stderr\n");
#ifdef DSD_USE_SERIALDV
    fprintf(stderr, "  -D <device>   Use DVSI AMBE3000 based device for AMBE decoding (e.g. ThumbDV)\n");
    fprintf(stderr, "                You must have compiled with serialDV support (see Readme.md)\n");
    fprintf(stderr, "                Device name is the corresponding TTY USB device e.g /dev/ttyUSB0\n");
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "Scanner control options:\n");
    fprintf(stderr,
            "  -R <num>      Resume scan after <num> TDULC frames or any PDU or TSDU\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Decoder options:\n");
    fprintf(stderr, "  -d <num>      Set data rate:");
    fprintf(stderr, "     0          2400 bauds");
    fprintf(stderr, "     1          4800 bauds (default)");
    fprintf(stderr, "     2          9800 bauds");
    fprintf(stderr, "  -fa           Auto-detect frame type (default)\n");
    fprintf(stderr, "  -fr           Decode only DMR/MOTOTRBO\n");
    fprintf(stderr, "  -fd           Decode only D-STAR\n");
    fprintf(stderr, "  -f1           Decode only P25 Phase 1\n");
    fprintf(stderr, "  -fi           Decode only NXDN48 (6.25 kHz) / IDAS*\n");
    fprintf(stderr, "  -fn           Decode only NXDN96 (12.5 kHz)\n");
    fprintf(stderr, "  -fp           Decode only ProVoice\n");
    fprintf(stderr, "  -fx           Decode only X2-TDMA\n");
    fprintf(stderr, "  -fm           Decode only DPMR Tier 1 or 2 (6.25 kHz)\n");
    fprintf(stderr, "  -fy           Decode only YSF\n");
    fprintf(stderr, "  -l            Disable DMR/MOTOTRBO and NXDN input filtering\n");
    fprintf(stderr, "  -pu           Unmute Encrypted P25\n");
    fprintf(stderr, "  -u <num>      Unvoiced speech quality (default=3)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Advanced decoder options:\n");
    fprintf(stderr,
            "  -A <num>      QPSK modulation auto detection threshold (default=26)\n");
    fprintf(stderr,
            "  -S <num>      Symbol buffer size for QPSK decision point tracking\n");
    fprintf(stderr, "                 (default=36)\n");
    fprintf(stderr,
            "  -M <num>      Min/Max buffer size for QPSK decision point tracking\n");
    fprintf(stderr, "                 (default=15)\n");
    exit(0);
}

void sigfun(int sig)
{
    exitflag = 1;
    signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
    int c;
    extern char *optarg;
    extern int optind, opterr, optopt;
    DSDcc::DSDDecoder dsdDecoder;
    DSDcc::DSDUpsampler upsamplingEngine;
    char in_file[1023];
    int  in_file_fd = -1;
    char out_file[1023];
    int  out_file_fd = -1;
    char log_file[1023];
    log_file[0] = '\0';
    char serialDevice[16];
    std::string dvSerialDevice;
    int dvGain_dB = 0;

    fprintf(stderr, "Digital Speech Decoder DSDcc\n");

    exitflag = 0;
    signal(SIGINT, sigfun);

    while ((c = getopt(argc, argv,
            "hep:qstv:z:i:o:g:nR:f:u:U:A:S:M:lL:D:d:")) != -1)
    {
        opterr = 0;
        switch (c)
        {
        case 'h':
            usage();
            exit(0);
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
        case 's':
            dsdDecoder.showDatascope();
            break;
        case 't':
            dsdDecoder.showSymbolTiming();
            break;
        case 'v':
            int verbosity;
            sscanf(optarg, "%d", &verbosity);
            dsdDecoder.setLogVerbosity(verbosity);
            break;
        case 'z':
            int frameRate;
            sscanf(optarg, "%d", &frameRate);
            dsdDecoder.setDatascopeFrameRate(frameRate);
            break;
        case 'L':
            strncpy(log_file, (const char *) optarg, 1023);
            log_file[1023] = '\0';
            break;
        case 'i':
            strncpy(in_file, (const char *) optarg, 1023);
            in_file[1023] = '\0';
            break;
        case 'o':
            strncpy(out_file, (const char *) optarg, 1023);
            out_file[1023] = '\0';
            break;
#ifdef DSD_USE_SERIALDV
        case 'D':
            strncpy(serialDevice, (const char *) optarg, 16);
            serialDevice[16] = '\0';
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
        case 'A':
            int threshold;
            sscanf(optarg, "%i", &threshold);
            dsdDecoder.setAutoDetectionThreshold(threshold);
            break;
        case 'S':
            int ssize;
            sscanf(optarg, "%i", &ssize);
            dsdDecoder.setQPSKSymbolBufferSize(ssize);
            break;
        case 'M':
            int msize;
            sscanf(optarg, "%i", &msize);
            dsdDecoder.setQPSKMinMaxBufferSize(msize);
            break;
        case 'l':
            dsdDecoder.enableCosineFiltering(false);
            break;
        default:
            usage();
            exit(0);
        }
    }

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
    unsigned char dvMbeSamples[SerialDV::MBE_FRAME_LENGTH_BYTES];

    if (!dvSerialDevice.empty())
    {
        if (dvController.open(dvSerialDevice))
        {
            fprintf(stderr, "SerialDV up. Disable mbelib support\n");
            dsdDecoder.enableMbelib(false); // de-activate mbelib decoding
        }
    }
#endif

    while (exitflag == 0)
    {
        short sample;
        int nbAudioSamples;
        short *audioSamples;

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
            if (dsdDecoder.mbeDVReady())
            {
                dvController.decode(dvAudioSamples, (const unsigned char *) dsdDecoder.getMbeDVFrame(), (SerialDV::DVRate) dsdDecoder.getMbeRate(), dvGain_dB);
                if (dsdDecoder.upsampling())
                {
                    upsamplingEngine.upsample(dsdDecoder.upsampling(), dvAudioSamples, &dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE], SerialDV::MBE_AUDIO_BLOCK_SIZE);
                    result = write(out_file_fd, (const void *) &dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE], SerialDV::MBE_AUDIO_BLOCK_BYTES * dsdDecoder.upsampling());
                }
                else
                {
                    result = write(out_file_fd, (const void *) dvAudioSamples, SerialDV::MBE_AUDIO_BLOCK_BYTES); // TODO: upsampling
                }
                dsdDecoder.resetMbeDV();
            }
        }
        else
#endif
        {
            audioSamples = dsdDecoder.getAudio1(nbAudioSamples);

            if (nbAudioSamples > 0)
            {
                result = write(out_file_fd, (const void *) audioSamples, sizeof(short) * nbAudioSamples);

                if (result == -1) {
                    fprintf(stderr, "Error writing to output\n");
                } else if (result != sizeof(short) * nbAudioSamples) {
                    fprintf(stderr, "Written %d out of %d audio samples\n", result/2, nbAudioSamples);
                }

                dsdDecoder.resetAudio1();
            }
        }
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

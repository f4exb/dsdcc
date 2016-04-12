<h1>Samples directory</h1>

<h2>Discriminator output samples</h2>

File extension: `.dis`

These are samples taken at the output of the discriminator of the radio. These are S16LE 48 kS/s single channel (mono) samples. 

They can be used as pipe input to `dsdccx` using `sox` utility as in this example: `sox -t s16 -r 48k -c 1 dmr_it_8.dis -t s16 -r 48k -c 1 - | /opt/install/dsdcc/bin/dsdccx -i - -fa -o - | play -q -t s16 -r 8k -c 1 -`

The files are:

  - `dmr_it_8.dis`: Example of a DMR signal. This is some technical conversation in Italian. 
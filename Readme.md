DSDcc
=====

**DSDcc** is a complete rewrite from the original [DSD (Digital Speech Decoder)](https://github.com/szechyjs/dsd) project.

It is rewritten along the following lines:

  - A purely C++ library with a single decoder object at its central point
  - Works by pushing new samples to the decoder at the upper level rather than pulling it from the underlying filesystem (actual file or device) at the lowest level. This facilitates integration with software using it as a true library. This comes especially handy for projects in Qt that cannot afford using pthreads on their own like [gr-dsd](https://github.com/argilo/gr-dsd) does. In fact the main drive for this is to integrate it in a plugin of [SDRangel](https://github.com/f4exb/sdrangel).
  - Works by polling to get possible new audio samples after new samples have been pushed to the decoder
  - Option to output audio samples as L+R (stereo) samples with L=R as this may facilitate integration
  - A binary that uses this library is provided for integration with other commands that run in a shell. So basically it works only with input / output files possibly being `stdin` / `stdout` to be integrated in a pipe command. There is no direct usage of audio devices nor fancy side reading from or writing to `.wav` or `.mbe` files.

These points have been retained from the original:

  - The decoding methods
  - Minimal changes to the options and state structures
  - `mbelib` usage (of course...) with the usual restrictions on possible copyright violations (See next)
  - Input as S16LE samples at a fixed rate of 48kS/s
  - Audio output as S16LE samples at 8kS/s rate directly out of `mbelib` or upsampled to 48kS/s

<h1>Possible copyright issues with mbelib</h1>

While DSDcc is intended to be patent-free, `mbelib` that it uses describes functions that may be covered by one or more U.S. patents owned by DVSI Inc. The source code itself should not be infringing as it merely describes possible methods of implementation. Compiling or using `mbelib` may infringe on patents rights in your jurisdiction and/or require licensing. It is unknown if DVSI will sell licenses for software that uses `mbelib`.

If you are not comfortable with this just do not compile or use this software.

<h1>Supported formats</h1>

These are only a subset of the ones covered by the original DSD project. Migration from original DSD is not easy and will be done little by little and also depends on the test material available. For now we have:

  - DMR/MOTOTRBO: European two slot TDMA standard. MOTOTRBO is a popular implementation of this standard.
  - D-Star: developed and promoted by Icom for Amateur Radio customers.

Next we might want to add YSF (Yaesu Sound Fusion a.k.a. C4FM) as a newly supported format to cover all AMBE codec based commercial formats used at present by Amateur Radio. YSF is not supported by the original DSD but there is a GNUradio [gr-ysf](http://hb9uf.github.io/gr-ysf/) module available.

<h1>Source code</h1>

<h2>Repository branches</h2>

- master: the production branch
- dev: the development branch

<h1>Building</h1>

As usual with projects based on cmake create a `build` directory at the root of the cloned repository and cd into it.

You will need [mbelib](https://github.com/szechyjs/mbelib) installed in your system. If you use custom installation paths like `/opt/install/mbelib` for example you will need to add the include and library locations to the cmake command line with these directives: `-DLIBMBE_INCLUDE_DIR=/opt/install/mbelib/include -DLIBMBE_LIBRARY=/opt/install/mbelib/lib/libmbe.so`

So the full cmake command with a custom installation directory will look like: `cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=/opt/install/dsdcc -DLIBMBE_INCLUDE_DIR=/opt/install/mbelib/include -DLIBMBE_LIBRARY=/opt/install/mbelib/lib/libmbe.so ..`

Then:

  - `make` or `make -j8` on a 8 CPU machine
  - `make install`

<h1>Running</h1>

A binary `dsdccx` is produced and gets installed in the `bin` subdirectory of your installation directory. A typical usage is to pipe in the input from a UDP source of discriminator output samples with `socat` and pipe out to `sox` `play` utility to produce some sound:
`socat stdout udp-listen:9999 | /opt/install/dsdcc/bin/dsdccx -i - -fa -o - | play -q -t s16 -r 8k -c 1 -`

For more details refer to the online help with the `-h` option: `dsdccx -h`

<h1>Developpers notes</h1>

<h2>Structure overview</h2>

  - Everything lives in the `DSDcc` namespace
  - The `DSDDecoder` object handles the core functions of synchronization and global orchestration of the decoding. It also hosts the options and state objects. It collaborates with specialized objects that have full access to the decoder public and private areas using the C++ `friend` directive.
  - The options and state objects are the following:
    - The `DSDOpts` object handles the options configuring the behaviour of the decoder
    - The `DSDState` object handles the run time data and data related to the current state of the decoder
  - The `DSDSymbol` object is responsible for symbol and dibit processing. It receives a new sample with its `pushSample()` method. It processes it and when enough samples have been receives it can produce a new symbol that it stores internally.
  - The `DSDMBEDecoder` object is responsible of taking in AMBE frames and producing the final audio output at 8 kS/s. It is a wrapper around the `mbelib` library. It also handles the optional upsampling of audio to 48 kS/s.
  - The objects specialized in the decoding of the various formats are:
    - The `DSDDMRVoice` object is responsible of handling the processing of DMR voice frames. It uses the service of `DSDMBEDecoder` to produce the final audio output.
    - The `DSDDMRData` object is responsible of handling the processing of DMR data frames. It does not produce any audio output.
    - The `DSDDstar` object is responsible of handling the processing of D-Star frames. It uses the service of `DSDMBEDecoder` to produce the final audio output.
  - Some utility objects are also defined:
    - The `Descramble` object contains static data and methods mainly used in the decoding of D-Star frames. It is based on Jonathan Naylor G4KLX code.
    - The `DSDFilters` object as the name implies contains methods to perform various forms of DSP filtering.

<h2>Typical integration</h2>

You can look at the source of the `dsdccx` binary to get an idea. Basically it involves the following steps:

  1. Allocate a new `DSDDecoder` object (stack or heap)
  2. Set the options and state object. with some `DSDDecoder` methods.
  3. Prepare the input (open file or stream)
  4. Get a new sample from the stream
  5. Push this sample to the decoder
  6. Check if any audio output is available and possibly get its pointer and number of samples
  7. Push these samples to the audio device or the output file or stream
  8. Go back to step #5 until a signal is received or some sort of logic brings the loop to an end
  9. Do the cleanup after the loop or in the signal handler (close file, destroy objects...)

Of course this loop can be run in its own thread or remain synchronous with the calling application. Unlike with the original DSD you have the choice.

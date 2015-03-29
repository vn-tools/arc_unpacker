A tool for extracting images and sounds from visual novels.

*Disclaimer*: this project doesn't support modifying the game files. If you'd
like to translate a game into English and need tools for game script/image
modification, contact me via the e-mail address listed in my [Github
profile](https://github.com/rr-).



Download
--------

To download binaries for Windows, head over to
[releases](https://github.com/vn-tools/arc_unpacker/releases).
For build instructions, see below.

Usage
-----

1. Drag and drop archive files to `arc_unpacker`.
2. Drag and drop non-archive files onto `file_decoder`.
3. Some formats need additional switches in order to unpack correctly.
   These can be found out using `--help`.



Supported games
---------------

If you believe there is an error in these tables, please let me know.

### Archives

CLI invocation                        | Game                                | Features<sup>1</sup>
------------------------------------- | ----------------------------------- | ------------
`--fmt=xp3 --plugin=noop`             | Sono Hanabira ni Kuchizuke o 12     | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=noop`             | Sharin no Kuni, Himawari no Shoujo  | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=fsn`              | Fate/Stay Night                     | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=fha`              | Fate/Hollow Ataraxia                | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=comyu`            | Comyu Kuroi Ryuu to Yasashii Oukoku | ![][sup]![][sup]![][sup]
`--fmt=p`                             | Melty Blood                         | ![][sup]![][sup]![][sup]
`--fmt=sar`                           | Tsukihime                           | ![][sup]![][sup]![][sup]
`--fmt=nsa`                           | Tsukihime                           | ![][sup]![][sup]![][sup]
`--fmt=nsa`                           | Umineko No Naku Koro Ni             | ![][sup]![][sup]![][sup]
`--fmt=ykc`                           | Hoshizora e Kakaru Hashi            | ![][sup]![][sup]![][sup]
`--fmt=fjsys`                         | Sono Hanabira ni Kuchizuke o 1-11   | ![][sup]![][sup]![][sup]
`--fmt=pak`                           | Saya no Uta                         | ![][sup]![][sup]![][sup]
`--fmt=npa`                           | Chaos;Head                          | ![][sup]![][sup]![][sup]
`--fmt=rpa`                           | Everlasting Summer                  | ![][sup]![][sup]![][sup]
`--fmt=rpa`                           | Long Live The Queen                 | ![][sup]![][sup]![][sup]
`--fmt=rpa`                           | Katawa Shoujo                       | ![][sup]![][sup]![][sup]
`--fmt=mbl`                           | Wanko to Kurasou                    | ![][sup]![][sup]![][sup]
`--fmt=rgssad`                        | Cherry Tree High Comedy Club        | ![][sup]![][sup]![][sup]
`--fmt=arc`                           | Higurashi No Naku Koro Ni           | ![][sup]![][sup]![][sup]
`--fmt=npa_sg`                        | Steins;Gate                         | ![][sup]![][sup]![][sup]
`--fmt=pac`                           | Maji de Watashi ni Koishinasai!     | ![][sup]![][sup]![][sup]
`--fmt=pack`                          | Soshite Ashita no Sekai yori        | ![][sup]![][sup]![][sup]
`--fmt=pack`                          | Kimihagu                            | ![][sup]![][sup]![][sup]
`--fmt=pack --fkey=... --gameexe=...` | Koiken Otome                        | ![][sup]![][sup]![][sup]
`--fmt=pbg3`                          | Touhou 06                           | ![][sup]![][sup]![][sup]
`--fmt=pbg4`                          | Touhou 07                           | ![][sup]![][sup]![][par]
`--fmt=th-pak1`                       | Touhou 07.5                         | ![][sup]![][sup]![][sup]
`--fmt=pbgz`                          | Touhou 08                           | ![][sup]![][sup]![][par]
`--fmt=pbgz`                          | Touhou 09                           | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 09.5                         | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 10                           | ![][sup]![][sup]![][par]
`--fmt=th-pak2`                       | Touhou 10.5                         | ![][sup]![][sup]![][sup]
`--fmt=tha1`                          | Touhou 11                           | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 12                           | ![][sup]![][sup]![][par]
`--fmt=th-pak2`                       | Touhou 12.3                         | ![][sup]![][sup]![][sup]
`--fmt=tha1`                          | Touhou 12.5                         | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 12.8                         | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 13                           | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 14                           | ![][sup]![][sup]![][par]
`--fmt=tha1`                          | Touhou 14.3                         | ![][sup]![][sup]![][par]
`--fmt=xfl`                           | Souten No Celenaria                 | ![][sup]![][sup]![][sup]
`--fmt=g2`                            | Musume Shimai                       | ![][sup]![][sup]![][sup]
`--fmt=gml`                           | Watashi no Puni Puni                | ![][sup]![][sup]![][sup]
`--fmt=exe`                           | Generic `.exe` resources            | ![][sup]![][nap]![][nap]

### Files

CLI invocation             | Game                                | Features<sup>1</sup>
-------------------------- | ----------------------------------- | ------------
`--fmt=g00`, `--fmt=nwa`   | Little Busters                      | ![][nap]![][sup]![][sup]
`--fmt=g00`, `--fmt=nwa`   | Clannad                             | ![][nap]![][sup]![][sup]
`--fmt=g00`, `--fmt=nwa`   | Kanon                               | ![][nap]![][sup]![][sup]
`--fmt=xyz`                | Yume Nikki                          | ![][nap]![][sup]![][sup]
`--fmt=sotes`              | Fortune Summoners                   | ![][nap]![][sup]![][sup]

<sup>1</sup> Feature legend:

- Generic files
- Graphics
- Sound and music

<sub>![][sup] - fully supported, ![][par] - partially supported, ![][non] -
unsupported, ![][nap] - doesn't apply.  
If generic file unpacking "doesn't apply", it means the game doesn't use
archives.</sub>

[sup]: http://i.imgur.com/PeYsbCg.png
[par]: http://i.imgur.com/NMBy1C0.png
[non]: http://i.imgur.com/2aTNlHb.png
[nap]: http://i.imgur.com/jQTmqxl.png

If the game isn't listed above, there is still a small chance that the files
can be extracted nonetheless. Note that some archives provide no way to verify
correctness of the extracted files, which means that there can be false
positives and you might get garbage files. That shouldn't happen with any of
the games listed above, though.



Building from sources
---------------------

### Requirements

1. `g++` 4.8+ or `mingw-w64`
2. `boost::locale`
3. `boost::filesystem`
4. `libpng` 1.4+
5. `zlib` (comes with `libpng`)
6. `iconv` (comes with POSIX)

### Building for Linux

Just download required dependencies, run `make` and you're good to go.

### Building for Windows on Cygwin

Just download required dependencies, run `make` and you're good to go. Note
that you'll need Cygwin on every computer you plan to run it on.

### Building for Windows without Cygwin dependencies

In this case you need to use `mingw-w64`. On Cygwin you can download most of
the dependencies from [Cygwin Ports](http://cygwinports.org/). On Linux, you
probably need to compile the dependencies yourself. Below I describe the manual
way which should work for both Cygwin and Linux.

---

:warning: **Before proceeding, run `export MINGW=$HOME/mingw`** (or another
directory of your choice). This directory will contain the compiled
dependencies. Choosing folder in `$HOME/` has two advantages:

- we don't need to `sudo` on Linux machines
- the build is sandboxed, so when we screw up we can just `rm -rf $MINGW` and
  start over without littering the `/usr`.

#### Compiling `zlib`

    # Obtain sources
    wget 'http://zlib.net/zlib-1.2.8.tar.gz'
    tar xzvf zlib-1.2.8.tar.gz
    cd zlib-1.2.8/

    # Compile
    make \
        -f win32/Makefile.gcc \
        CC=i686-w64-mingw32-gcc \
        RC=i686-w64-mingw32-windres

    # Install
    make \
        -f win32/Makefile.gcc \
        install \
        BINARY_PATH=$MINGW/bin \
        INCLUDE_PATH=$MINGW/include \
        LIBRARY_PATH=$MINGW/lib \
        SHARED_MODE=1

#### Compiling `libiconv`

    # Obtain sources
    wget 'http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz'
    tar xzvf libiconv-1.14.tar.gz
    cd libiconv-1.14

    # Compile
    ./configure \
        --host=i686-w64-mingw32 \
        --prefix=$MINGW \
        CPPFLAGS=-I$MINGW/include \
        LDFLAGS=-L$MINGW/lib
    make

    # Install
    make install

#### Compiling `libpng`

    # Obtain sources
    wget 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.16.tar.gz'
    tar xzvf libpng-1.6.16.tar.gz
    cd libpng-1.6.16/

    # Compile
    ./configure \
        --host=i686-w64-mingw32 \
        --prefix=$MINGW \
        CPPFLAGS=-I$MINGW/include \
        LDFLAGS=-L$MINGW/lib
    make

    # Install
    make install

#### Compiling `boost`

    # Obtain sources
    wget 'http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.zip'
    unzip boost_1_57_0.zip
    cd boost_1_57_0

    # Compile and install
    ./bootstrap.sh --prefix=$MINGW
    echo "using gcc : mingw32 : /usr/bin/i686-w64-mingw32-g++ ;" > user-config.jam

    ./b2 \
        -j 4 \
        --layout=system \
        --user-config=user-config.jam \
        --with-locale \
        --with-filesystem \
        cxxflags="-I$MINGW/include" \
        linkflags="-L$MINGW/lib" \
        boost.locale.icu=off \
        boost.locale.winapi=off \
        boost.locale.iconv=on \
        variant=release \
        link=shared \
        runtime-link=shared \
        toolset=gcc-mingw32 \
        target-os=windows \
        threadapi=win32 \
        install

#### Compiling `OpenSSL`

    # Obtain sources
    wget 'http://openssl.org/source/openssl-1.0.2a.tar.gz'
    tar zxvf openssl-1.0.2a.tar.gz
    cd openssl-1.0.2a

    # Compile and install
    ./Configure \
        --cross-compile-prefix=i686-w64-mingw32- \
        --prefix=$MINGW \
        mingw64 no-asm shared
    make # (parallelizing with -j may break the build)

    # Install
    make install

This will run a minimal required build (thanks to `--with` options), so it
shouldn't take too long.

---

Finally, run `./make-mingw-w64.sh`. Then locate the compiled DLL-s in either
`/usr` (`libstdc++` etc) or `$MINGW` (`libboost` etc):

- `libboost_filesystem.dll`
- `libboost_locale.dll`
- `libboost_system.dll`
- `libgcc_s_sjlj-1.dll`
- `libiconv-2.dll`
- `libpng16-16.dll`
- `libstdc++-6.dll`
- `libeay32.dll`

...and copy them into the binaries directory. Now the program should be ready
to ship.

### Building in Visual Studio

I'm open to pull requests.

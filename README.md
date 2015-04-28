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

Drag'n'drop the archive or file onto `arc_unpacker`. It will guess the format
and unpack it for you.

Caveats:

1. The file format might be detected by two or more decoders at once. In such
cases you need to look at the console output, choose the valid format and tell
the program to use it by supplying `--fmt=...` option.

2. The file might need more parameters to be correctly unpacked. In such cases
you need to supply `--fmt=FORMAT` option and additional parameters. For
example, XP3 archives need `--plugin` that tells what kind of decryption to
use. To tell it to use Fate/Stay Night decryption, supply `--fmt=xp3
--plugin=fsn`. All of these options along with their values can be discovered
with `--fmt=FORMAT --help` switch.



Supported games
---------------

Feature legend:

![legend](https://cloud.githubusercontent.com/assets/1045476/7106067/af96459a-e135-11e4-956b-bc733b1e4ab2.png)

### Archives

Features           | Game                                | CLI invocation
------------------ | ----------------------------------- | --------------
![][F]![][X]![][X] | Generic `.exe` resources            | `--fmt=exe`
![][F]![][F]![][F] | Chaos;Head                          | `--fmt=npa`
![][F]![][F]![][F] | Cherry Tree High Comedy Club        | `--fmt=rgssad`
![][F]![][F]![][F] | Comyu Kuroi Ryuu to Yasashii Oukoku | `--fmt=xp3 --plugin=comyu`
![][F]![][F]![][F] | Everlasting Summer                  | `--fmt=rpa`
![][F]![][F]![][F] | Fate/Hollow Ataraxia                | `--fmt=xp3 --plugin=fha`
![][F]![][F]![][F] | Fate/Stay Night                     | `--fmt=xp3 --plugin=fsn`
![][F]![][F]![][F] | Higurashi No Naku Koro Ni           | `--fmt=arc`
![][F]![][F]![][F] | Hoshizora e Kakaru Hashi            | `--fmt=ykc`
![][F]![][F]![][F] | Irotoridori no Sekai                | `--fmt=fvp`
![][F]![][F]![][F] | Katawa Shoujo                       | `--fmt=rpa`
![][F]![][F]![][F] | Kimihagu                            | `--fmt=pack`
![][F]![][F]![][F] | Koiken Otome                        | `--fmt=pack --fkey=... --gameexe=...`
![][F]![][F]![][F] | Long Live The Queen                 | `--fmt=rpa`
![][F]![][F]![][F] | Maji de Watashi ni Koishinasai!     | `--fmt=pac`
![][F]![][F]![][F] | Melty Blood                         | `--fmt=p`
![][F]![][F]![][F] | Musume Shimai                       | `--fmt=g2`
![][F]![][F]![][F] | Saya no Uta                         | `--fmt=pak`
![][F]![][F]![][F] | Sharin no Kuni, Himawari no Shoujo  | `--fmt=xp3 --plugin=noop`
![][F]![][F]![][F] | Sono Hanabira ni Kuchizuke o 1-11   | `--fmt=fjsys`
![][F]![][F]![][F] | Sono Hanabira ni Kuchizuke o 12     | `--fmt=xp3 --plugin=noop`
![][F]![][F]![][F] | Soshite Ashita no Sekai yori        | `--fmt=pack`
![][F]![][F]![][F] | Souten No Celenaria                 | `--fmt=xfl`
![][F]![][F]![][F] | Steins;Gate                         | `--fmt=npa_sg`
![][F]![][F]![][F] | Touhou 06                           | `--fmt=pbg3`
![][F]![][F]![][P] | Touhou 07                           | `--fmt=pbg4`
![][F]![][F]![][F] | Touhou 07.5                         | `--fmt=th-pak1`
![][F]![][F]![][P] | Touhou 08                           | `--fmt=pbgz`
![][F]![][F]![][P] | Touhou 09                           | `--fmt=pbgz`
![][F]![][F]![][P] | Touhou 09.5                         | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 10                           | `--fmt=tha1`
![][F]![][F]![][F] | Touhou 10.5                         | `--fmt=th-pak2`
![][F]![][F]![][P] | Touhou 11                           | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 12                           | `--fmt=tha1`
![][F]![][F]![][F] | Touhou 12.3                         | `--fmt=th-pak2`
![][F]![][F]![][P] | Touhou 12.5                         | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 12.8                         | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 13                           | `--fmt=tha1`
![][F]![][F]![][F] | Touhou 13.5                         | `--fmt=tfpk`
![][F]![][F]![][P] | Touhou 14                           | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 14.3                         | `--fmt=tha1`
![][F]![][F]![][F] | Tsukihime                           | `--fmt=nsa`, `--fmt=sar`
![][F]![][F]![][F] | Umineko No Naku Koro Ni             | `--fmt=nsa`
![][F]![][F]![][F] | Wanko to Kurasou                    | `--fmt=mbl`
![][F]![][F]![][F] | Watashi no Puni Puni                | `--fmt=gml`

### Files

Features           | Game                                | CLI invocation
------------------ | ----------------------------------- | --------------
![][X]![][F]![][F] | Clannad                             | `--fmt=g00`, `--fmt=nwa`
![][X]![][F]![][F] | Fortune Summoners                   | `--fmt=sotes`
![][X]![][F]![][F] | Kanon                               | `--fmt=g00`, `--fmt=nwa`
![][X]![][F]![][F] | Little Busters                      | `--fmt=g00`, `--fmt=nwa`
![][X]![][F]![][F] | Yume Nikki                          | `--fmt=xyz`

[F]: http://i.imgur.com/PeYsbCg.png
[P]: http://i.imgur.com/NMBy1C0.png
[N]: http://i.imgur.com/2aTNlHb.png
[X]: http://i.imgur.com/jQTmqxl.png

If a game isn't listed above and it works, please let me know so I can update
the table.



---

Building from sources
---------------------

### Requirements

1. `g++` 4.8+ or `mingw-w64`
2. `boost::locale`
3. `boost::filesystem`
4. `libpng` 1.4+
5. `zlib` (comes with `libpng`)
6. `iconv` (comes with POSIX)
7. `openssl`

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

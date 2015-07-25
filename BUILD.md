Build instructions
------------------

### Dependencies

Required:

1. `g++` 4.8+ or `mingw-w64`
2. `boost::locale`
3. `boost::filesystem`
4. `libpng` 1.4+
5. `zlib` (comes with `libpng`)
6. `iconv` (comes with POSIX)

Recommended:

1. `openssl`

### Compiling for GNU/Linux or Cygwin

1. Download required dependencies (the ones with `-dev` suffix). On Cygwin most
   of them are available on [Cygwin Ports](http://cygwinports.org/).
2. Run following:

        ./bootstrap
        ./waf configure
        ./waf build

3. The executables should appear in `build/` directory.

### Cross compiling for Windows

In this case, the project requires MinGW-w64. There are several ways to install
it, but I believe using [`mxe`](http://mxe.cc/) is the most pleasant way.

1. Install `mxe`:

        git clone https://github.com/mxe/mxe.git

2. Build `arc_unpacker`'s dependencies using `mxe`:

        cd mxe
        make libiconv
        make zlib
        make libpng
        make boost
        make openssl

3. Configure the shell to use `mxe`:

        MXE_PATH=~/path/to/mxe
        CROSS=i686-w64-mingw32.static-
        export PATH="$MXE_PATH/usr/bin/:$PATH"
        export CC=${CROSS}gcc
        export CXX=${CROSS}g++
        export AR=${CROSS}ar
        export PKGCONFIG=${CROSS}pkg-config

3. In the `arc_unpacker`'s directory run following:

        ./bootstrap
        ./waf configure
        ./waf build

### Compiling for Windows with Visual Studio

I'm open to pull requests.

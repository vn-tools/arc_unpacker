arc\_unpacker - the visual novel extractor
==========================================

`arc_unpacker` is a command line tool for extracting images, sounds, music and
miscellaneous resources from visual novels (and some other Japanese games).

- [**List of supported
  games**](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm)
- [**Latest stable
  release**](https://github.com/vn-tools/arc_unpacker/releases)
- [Build instructions for Windows and
  GNU/Linux](https://github.com/vn-tools/arc_unpacker/blob/master/BUILD.md)

## Usage

Drag and drop the archive or file onto `arc_unpacker`. It will guess the format
and unpack it for you.

Caveats:

1. The file format might be detected by two or more decoders at once. In such
   cases you need to tell the program which one to use by supplying `--dec=...`
   option.

2. The file might need more parameters to be correctly unpacked. In such cases
   you need to supply them manually. For example, XP3 archives need `--plugin`
   that tells what kind of decryption to use. To tell it to use Fate/Stay Night
   decryption, supply `--dec=krkr/xp3 --plugin=fsn`.

To learn what parameter your game needs, you can either use `--help` to see all
available parameters, or refer to your game details in the [game
list](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm) that lists
all required parameters for every supported game.

To learn how to pass parameters to `arc_unpacker`, refer to [this
question](#user-content-how-do-i-pass-additional-options--parameters).

## Q&A

- ##### I drag the game files onto `arc_unpacker` and it immediately closes.

  Try running it from the command prompt to see the output and possible error
  message. Usually this behavior is a sign that the game requires additional
  parameters which are described in the [game
  list](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm), or it's
  not supported.

- ##### What is command prompt?
  On Windows, it's `cmd.exe`. On Linux, I think you already know the answer to
  this question.

- ##### How do I pass additional options / parameters?

    *(for Windows users)*

    1. Open up `cmd.exe` and navigate to the directory where you downloaded
       `arc_unpacker.exe` by typing a command that looks like this:

            cd "C:\downloads"

       Alternatively, navigate to the `arc_unpacker` directory in Explorer,
       hold Shift and click with right mouse button anywhere, and select "Open
       command window here".

    2. To run `arc_unpacker` with your game without extra parameters, type a
       command that looks like this:

            arc_unpacker "C:\games\your game\file you want to unpack.dat"

        or

            arc_unpacker "C:\games\your game\directory you want to unpack"

       Alternatively, type `arc_unpacker ` (without hitting <kbd>Enter</kbd>)
       and drop a file onto command prompt. It should enter the path like
       above, saving you the effort of typing it manually.

    3. To pass additional parameters such as `--dec` etc., run `arc_unpacker`
       like this:

            arc_unpacker "C:\games\Touhou 08\th08.dat" --dec=team-shanghai-alice/pbgz

       To get list of possible parameters, see `arc_unpacker --help`. If your
       game needs any extra parameters, they're also outlined inside [the game
       list](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm).

- ##### Why command line? Why no windows / GUI?

  It'd take a lot of effort to make GUI for `arc_unpacker`:

  - cross platform - I'd need to use Qt or Gtk, and this adds to the project
    complexity (consider how we support five different compilers).
  - design - it's difficult to make a GUI that caters to most of use cases
    (recursive unpacking, selecting decoder, passing game key etc.)
  - effort - implementing it easily scales to dozens, if not hundreds of man
    hours. I'd rather focus on supporting more games.

  That being said, it's not entirely impossible for this to happen in the
  future. Once C++17 comes out, I plan to ditch some of the non-conforming
  compilers, which should ease things up a bit.

- ##### What do I do with `.wavloop` files?

  These files are audio loops. You can play them like normal `.wav`,
  however, most of the players will ignore looping information that is
  contained in such files. If you're looking for players that do support looped
  `.wav`s, I recommend either
  [`vgmstream`](https://github.com/kode54/vgmstream/),
  [`foo_input_wave_loop`](http://www.slemanique.com/software/foo_input_wave_loop.html)
  or [`wavosaur`](http://www.wavosaur.com/). The extension `.wavloop` was
  chosen so that it stands out from normal `.wav`s and for compatibility with
  `foo_input_wave_loop`.

- ##### I get `std::bad_alloc`, what gives?

  One option is that an archive contains a very large file, which causes
  `arc_unpacker` to run out of RAM while it tries to decode that file. To
  circumvent this (short of buying more memory), you can try running
  `arc_unpacker` with `-t=1 --no-recurse` which should reduce its memory
  footprint. Since there only were a few such archives spotted, no special
  mechanism was developed to work around this issue, although this might change
  in the future. Other options include corrupt game files or a specific kind of
  bug in `arc_unpacker`'s decoders, but both are unlikely. If you are unable to
  unpack the files, do not hesitate to report the issue to the issue tracker.

- ##### How can I help with development?

  Thanks for asking this! There are a number of ways you can help:

  - By documenting games that are already supported but are not present on the
    games list.
  - By porting existing decoders from other projects. Example projects can be
    found in THANKS.md.
  - By reverse engineering games that are not yet supported, and coding new
    decoders for them.

  For details, check out
  [`CONTRIBUTING.md`](https://github.com/vn-tools/arc_unpacker/blob/master/CONTRIBUTING.md).

- ##### Packing / encoding support?

  Not going to happen. Compiling files for games, especially from fan
  translation standpoint, almost always needs rolling your own script compiler,
  modifying the game .exe, fixing weird quirks, etc. - many things that
  wouldn't make sense for `arc_unpacker` to support. At the same time, the user
  base that might find encoders useful would be extremely small, and writing
  encoders is often much more difficult than writing decoders, so it's not very
  practical.

## Contact

- Bug reporting: [GitHub issue
  tracker](https://github.com/vn-tools/arc_unpacker/issues)
- Game requests: [GitHub issue
  tracker](https://github.com/vn-tools/arc_unpacker/issues)

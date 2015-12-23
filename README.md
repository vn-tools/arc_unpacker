arc\_unpacker
=============

A command line tool for extracting images and sounds from visual novels.

## Installation

- Unstable Windows binaries: [nightly builds](http://tmp.sakuya.pl/au/)
- Stable Windows binaries:
  [Github releases](https://github.com/vn-tools/arc_unpacker/releases)
- Build instructions for Windows and GNU/Linux:
  [`BUILD.md`](https://github.com/vn-tools/arc_unpacker/blob/master/BUILD.md)

## Supported games

See
[`GAMELIST.htm`](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm)
(the table is held in a separate HTML document due to space limitations of
GitHub).

## Usage

Drag'n'drop the archive or file onto `arc_unpacker`. It will guess the format
and unpack it for you.

Caveats:

1. The file format might be detected by two or more decoders at once. In such
   cases you need to tell the program which one to use by supplying `--fmt=...`
   option.

2. The file might need more parameters to be correctly unpacked (all of which
   can be discovered with `--help --fmt=FORMAT` switch). In such cases you need
   to supply them manually. For example, XP3 archives need `--plugin` that
   tells what kind of decryption to use. To tell it to use Fate/Stay Night
   decryption, supply `--fmt=krkr/xp3 --plugin=fsn`.

## Q&A

- **What do I do with `.wavloop` files?**  
  These files are audio loops. You can play them like normal `.wav`,
  however, most of the players will ignore looping information that is
  contained in such files. If you're looking for players that do support looped
  `.wav`s, I recommend either
  [vgmstream](https://github.com/kode54/vgmstream/),
  [`foo_input_wave_loop`](http://www.slemanique.com/software/foo_input_wave_loop.html)
  or [`wavosaur`](http://www.wavosaur.com/).
  The extension `.wavloop` was chosen so that it stands out from normal `.wav`s
  and for compatibility with `foo_input_wave_loop`.

## License

All the code is licensed under
[`LICENSE.md`](https://github.com/vn-tools/arc_unpacker/blob/master/LICENSE.md)
unless otherwise noted. For acknowledgments, see
[`THANKS.md`](https://github.com/vn-tools/arc_unpacker/blob/master/THANKS.md).

## Contact

- Bug reporting: [GitHub issue
  tracker](https://github.com/vn-tools/arc_unpacker/issues)
- Game requests, casual discussion: IRC channel - `#arc_unpacker` on Rizon
- Help with translation projects: my e-mail address listed in my [Github
  profile](https://github.com/rr-)

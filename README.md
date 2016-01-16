arc\_unpacker - the visual novel extractor
==========================================

`arc_unpacker` is a command line tool for extracting images, sounds, music and
miscellaneous resources from visual novels (and some other Japanese games).

- [**List of supported
  games**](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm)
- [**Nightly builds for Windows**](http://tmp.sakuya.pl/au/)
- [**Latest stable
  release**](https://github.com/vn-tools/arc_unpacker/releases)
- [Build instructions for Windows and
  GNU/Linux](https://github.com/vn-tools/arc_unpacker/blob/master/BUILD.md)

## Usage

Drag'n'drop the archive or file onto `arc_unpacker`. It will guess the format
and unpack it for you.

Caveats:

1. The file format might be detected by two or more decoders at once. In such
   cases you need to tell the program which one to use by supplying `--dec=...`
   option.

2. The file might need more parameters to be correctly unpacked (all of which
   can be discovered with `--help --dec=DECODER` switch). In such cases you
   need to supply them manually. For example, XP3 archives need `--plugin` that
   tells what kind of decryption to use. To tell it to use Fate/Stay Night
   decryption, supply `--dec=krkr/xp3 --plugin=fsn`.

## Q&A

- **What do I do with `.wavloop` files?**  
  These files are audio loops. You can play them like normal `.wav`,
  however, most of the players will ignore looping information that is
  contained in such files. If you're looking for players that do support looped
  `.wav`s, I recommend either
  [`vgmstream`](https://github.com/kode54/vgmstream/),
  [`foo_input_wave_loop`](http://www.slemanique.com/software/foo_input_wave_loop.html)
  or [`wavosaur`](http://www.wavosaur.com/). The extension `.wavloop` was
  chosen so that it stands out from normal `.wav`s and for compatibility with
  `foo_input_wave_loop`.

- **What's the license?**  
  All the code is licensed under
  [`LICENSE.md`](https://github.com/vn-tools/arc_unpacker/blob/master/LICENSE.md)
  unless otherwise noted. For acknowledgments, see
  [`THANKS.md`](https://github.com/vn-tools/arc_unpacker/blob/master/THANKS.md).

## Contact

- Bug reporting: [GitHub issue
  tracker](https://github.com/vn-tools/arc_unpacker/issues)
- Game requests: [GitHub issue
  tracker](https://github.com/vn-tools/arc_unpacker/issues)
- IRC channel: `#arc_unpacker` on `irc.rizon.net`

I can also help with translation projects - drop an email at rr-@sakuya.pl.

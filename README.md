Overview
--------

`arc_unpacker` is a tool for extracting images and sounds from visual novels.

*Disclaimer*: this project doesn't support modifying the game files. If you'd
like to translate a game into English and need tools for game script/image
modification, contact me via the e-mail address listed in my [Github
profile](https://github.com/rr-).



Download
--------

- [Windows - releases](https://github.com/vn-tools/arc_unpacker/releases)
- [Windows - nightly builds](http://tmp.sakuya.pl/au/)
- [Build instructions](https://github.com/vn-tools/arc_unpacker/blob/master/BUILD.md)  
  Supported platforms: Windows, Cygwin and GNU/Linux.

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

See [GAMELIST.htm](https://rawgit.com/vn-tools/arc_unpacker/master/GAMELIST.htm)
(the table is held in a separate HTML document due to space limitations of
GitHub).

License
-------

See
[LICENSE.md](https://github.com/vn-tools/arc_unpacker/blob/master/LICENSE.md).



Contact
-------

To report issues, use [GitHub issue
tracker](https://github.com/vn-tools/arc_unpacker/issues).  
For more casual discussion, hit the IRC channel - `#arc_unpacker` on Rizon.

A tool for extracting data from visual novels.

This branch is deprecated: for performance reasons the project was translated
to C++ (see issue #5).

Requirements
------------

1. `ruby` interpreter (tested on 2.0.0p598)
2. `rmagick` gem
3. `gcc`.

Before running the script, please run `sh build.sh` to build native extensions.
Otherwise, some formats might not work correctly.

Usage
-----

1. Choose the correct --fmt parameter to decrypt the files properly.
2. Run this script on the archive files. These can be found in the game's
   installation folder.

#### Example

    ruby ./bin/arc_unpacker.rb --fmt=xp3 --plugin=fsn ~/games/fate/fgimage.xp3 ./fgimage_unpacked/

Supported games
---------------

CLI invocation                 | Game                                | Features<sup>1</sup>
------------------------------ | ----------------------------------- | ------------
`--fmt=xp3 --plugin=none`      | Sono Hanabira ni Kuchizuke o 12     | ![][sup]![][sup]![][sup]![][non]
`--fmt=xp3 --plugin=none`      | Sharin no Kuni, Himawari no Shoujo  | ![][sup]![][sup]![][sup]![][non]
`--fmt=xp3 --plugin=fsn`       | Fate/Stay Night                     | ![][sup]![][sup]![][sup]![][non]
`--fmt=xp3 --plugin=fha`       | Fate/Hollow Ataraxia                | ![][sup]![][sup]![][sup]![][non]
`--fmt=xp3 --plugin=fha`       | Comyu Kuroi Ryuu to Yasashii Oukoku | ![][sup]![][sup]![][sup]![][non]
`--fmt=melty_blood`            | Melty Blood                         | ![][sup]![][non]![][non]![][non]
`--fmt=sar`                    | Tsukihime                           | ![][sup]![][sup]![][sup]![][non]
`--fmt=nsa`                    | Tsukihime                           | ![][sup]![][sup]![][sup]![][non]
`--fmt=nsa`                    | Umineko No Naku Koro Ni             | ![][sup]![][sup]![][sup]![][non]
`--fmt=ykc`                    | Hoshizora e Kakaru Hashi            | ![][sup]![][sup]![][sup]![][sup]
`--fmt=fjsys --key=sonohana1`  | Sono Hanabira ni Kuchizuke o 1      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana2`  | Sono Hanabira ni Kuchizuke o 2      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana3`  | Sono Hanabira ni Kuchizuke o 3      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana4`  | Sono Hanabira ni Kuchizuke o 4      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana5`  | Sono Hanabira ni Kuchizuke o 5      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana6`  | Sono Hanabira ni Kuchizuke o 6      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana7`  | Sono Hanabira ni Kuchizuke o 7      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana8`  | Sono Hanabira ni Kuchizuke o 8      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana9`  | Sono Hanabira ni Kuchizuke o 9      | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana10` | Sono Hanabira ni Kuchizuke o 10     | ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana11` | Sono Hanabira ni Kuchizuke o 11     | ![][sup]![][sup]![][sup]![][non]
`--fmt=nitroplus/pak2`         | Saya no Uta                         | ![][sup]![][sup]![][sup]![][non]
`--fmt=npa`                    | Chaos;Head                          | ![][sup]![][sup]![][sup]![][non]
`--fmt=rpa`                    | Everlasting Summer                  | ![][sup]![][sup]![][sup]![][non]
`--fmt=rpa`                    | Long Live The Queen                 | ![][sup]![][sup]![][sup]![][non]
`--fmt=rpa`                    | Katawa Shoujo                       | ![][sup]![][sup]![][sup]![][non]
`--fmt=mbl`                    | Wanko to Kurasou                    | ![][sup]![][sup]![][sup]![][non]
`--fmt=rgssad`                 | Cherry Tree High Comedy Club        | ![][sup]![][sup]![][sup]![][non]
`--fmt=arc`                    | Higurashi No Naku Koro Ni           | ![][sup]![][sup]![][non]![][non]
`--fmt=npa_sg`                 | Steins;Gate                         | ![][sup]![][sup]![][sup]![][non]
`--fmt=exe`                    | Generic `.exe` resources            | ![][sup]![][nap]![][nap]![][nap]
`--fmt=g00`, `--fmt=nwa`       | Little Busters                      | ![][nap]![][sup]![][sup]![][non]
`--fmt=g00`, `--fmt=nwa`       | Clannad                             | ![][nap]![][sup]![][sup]![][non]
`--fmt=g00`, `--fmt=nwa`       | Kanon                               | ![][nap]![][sup]![][sup]![][non]
`--fmt=xyz`                    | Yume Nikki                          | ![][nap]![][sup]![][sup]![][non]
`--fmt=sotes`                  | Fortune Summoners                   | ![][nap]![][sup]![][sup]![][non]

<sup>1</sup> Feature legend:

- Generic files
- Graphics
- Music
- Scripts

<sub>Graphics, music and scripts are listed separately, because each of them
could use encryption, compression or a niche file format. For example, being
able to unpack generic files doesn't mean you will be able to read TLG files
used by the game, unless `arc_unpacker` converts them specifically to
PNG.</sub>

<sub>![][sup] - fully supported, ![][par] - partially supported, ![][non] -
unsupported, ![][nap] - doesn't apply.  
If generic file unpacking "doesn't apply", it means the game doesn't use
archives.</sub>

[sup]: http://i.imgur.com/PeYsbCg.png
[par]: http://i.imgur.com/NMBy1C0.png
[non]: http://i.imgur.com/2aTNlHb.png
[nap]: http://i.imgur.com/jQTmqxl.png

#### Disclaimer

Some archives provide no way to verify correctness of the extracted files. This
means that for some unsupported games, the script may extract the files and
show no errors, but you will be unable to open them nonetheless.

Enabling drag'n'drop on Windows
-------------------------------

The unpacker is made so that it can accept just the path to the archive. To be
able to drag'n'drop the files onto the `bin/arc_unpacker.rb`, following
Registry patch should help:

    Windows Registry Editor Version 5.00

    ;The part that points to Ruby interpreter.
    [HKEY_CLASSES_ROOT\.rb]
    @="rb_file"

    [HKEY_CLASSES_ROOT\rb_file]
    @="Ruby script"

    [HKEY_CLASSES_ROOT\rb_file\shell]

    [HKEY_CLASSES_ROOT\rb_file\shell\open]

    [HKEY_CLASSES_ROOT\rb_file\shell\open\command]
    @="\"C:\\Ruby21-x64\\bin\\ruby.exe\" -- \"%1\" %*"
    ;note the "%*" - it's used to handle additional arguments.

    ;The part that tells Explorer to handle drag'n'drop event
    [HKEY_CLASSES_ROOT\rb_file\ShellEx]

    [HKEY_CLASSES_ROOT\rb_file\ShellEx\DropHandler]
    @="{60254CA5-953B-11CF-8C96-00AA00B8708C}"

<sub>If you're using Cygwin, to avoid problems with Unicode please make sure
you run the script either in a shell that supports UTF-8 (e.g. MinTTY), or use
official non-Cygwin Ruby installation. Trying to unpack files with non-ASCII
characters in their names within a `cmd.exe` using Cygwin's version of Ruby
interpreter may result in messed up names, or even exceptions during outputting
information to the console. `chcp 65001` does not help either. (This advice
applies to every Cygwin program, not just this project.)</sub>

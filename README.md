A tool for extracting data from XP3 archives used by Kirikiri engine.

Currently supported decryption routines:

- Noop (no encryption)
- Fate/Stay Night
- CXDEC-based:
  - Fate/Hollow Ataraxia

Usage:

1. `gem install bundler && bundle install`.
2. Choose the correct --fmt parameter to decrypt the files properly.
3. Run this script on the XP3 archives. These can be found in the game's
installation folder.

Note:

Unfortunately, I don't know any way to verify correctness of the extracted
files. This means that for the games using unsupported encryption, the script
won't notify you about possible decryption failures.

Contributing to `arc_unpacker`
==============================

New decoders, bug fixes, game documentation and more can be contributed via
pull requests and/or issues. All contributions are welcome.

### Code submissions

Please send a pull request to the `master` branch. Please include tests for new
decoders. Feel free to ask questions via issues.

##### Flow

- Fork the repository.
- Create a branch from `master`.
- Make the changes.
- Run the tests and the `checkstyle` script.
- Create a pull request to pull the changes from your branch to the `master`.

##### Guidelines

- Please consult other files and commit messages to maintain consistency.
- Changes to list of supported games should be separate from decoder additions.
- Please provide tests for any newly added decoders.
    - Tests for simple decoders should prepare test files with regular C++
      code. This simplifies adding new tests at a later time, should the
      decoder change.
    - Tests for complicated decoders may use binary files, but their size
      should be kept minimal. If it's possible, try to reuse existing assets.

##### Style convention

General rules:

- 80 characters per line, 4 spaces in indentation, no trailing whitespace
- each file should end with a newline
- when wrapping long function calls, each argument should be in separate line
  (including the first one)
- prefer `algo::range` over `for (int i = 0; i < ...; i++)`
- 50/72 characters in commit messages (with exception of changes to game list)

##### Gotchas

- The tests must be run from within repository root directory rather than from
  within the `build/` directory. Same goes for `tools/checkstyle`.
- `fmt` field in the game list contains approximate description with no
  particular convention - sometimes it uses magic, sometimes it uses file
  extensions, depending on which one is more recognizable.

### Requests for support of new games

Please provide:

- The game title in English or ローマ字 (if it's available).
- The game title in the original language (usually Japanese).
- A link to VNDB.
    - Alternatively, producer info and game's release date.

### Reporting bugs

Please provide (where it makes sense):

- What did you do?
- What did you expect to happen?
- What actually happened?
- What version of `arc_unpacker` are you using?
- What platform are you on?

Additionally, in case of crashes related to certain files, please provide a
minimal file for which the program crashes.

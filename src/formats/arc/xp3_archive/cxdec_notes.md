CXDEC is a third-party XP3 filter that games can use to protect their assets.
Its code usually resides in a `.tpm` file, which is just a DLL. However, static
analysis is discouraged - CXDEC seems to load most of its code at runtime.

During writing this I attached the debugger at the end of XP3 loading routine
(this can be easily found by searching for usages of `adlr` string constant -
it should precisely pinpoint XP3 loading routine), and then started the
analysis at the first call to the `.tpm` module.

Background info: CXDEC assembles raw machine code in a memory buffer. Then it
executes this code, deriving the final key used to decrypt the assets. The way
the code is assembled depends on the game and is executed in multiple stages.
Final decryption is emulated in `cxdec.rb`, and key derivation / code assembly
is emulated in `cxdec_key_deriver.rb`.

---

The games can control behavior of CXDEC with following elements:

- Two words, used to determine encryption block length.

- A 4 kiB table, used to do some substitutions during the code assembling.

- Finally, three permutations determining the order of blocks that will be
  produced by the assembler.

  First permutation is of length 3.
  Second permutation is of length 6.
  The last permutation is of length 8.

How to find these?

1. The two words can be found by searching for `and edi, 1234h` followed by
   `add edi, 5678h` in the `.tpm` module. `1234h` and `5678h` are the values we
   are looking for.

2. The table is located in the raw `.tpm` file, starts with `20 45 6e` and is
   4096 bytes long. No need for debugger, plain hex editor will do.

3. To find the first permutation, we need to hunt down a code like this in the
   `.tpm` module:

        xor edx, edx
        mov ecx, 3
        div ecx
        sub edx, 0
        jz JUMP1
        dec edx
        jz JUMP2
        dec edx
        jnz JUMP3
        push be

   Notice the `mov ecx, 3; div ecx`. It is roughly equivalent to the
   following:

        routine_number %= 3
        routine[routine_number].call

   We're going to use it in a sec.

   Heads up: CXDEC does not use any permutations. Everything is hardcoded - just
   like you can see above. However, the order of `JUMP1`, `JUMP2` and `JUMP3`
   is different for each game. Now, we have two options how to implement the
   decryptors for this. The first approach is to duplicate this switch and each
   piece of relevant code assembler in every plugin; `Crass` did this and it
   looked ugly and bloated. The other, smarter, option is to store only the
   order of these `JUMP`s. To do this, we need to assume some initial order,
   e.g. what `JUMP` is the "first", what is the "second" and what is the
   "third". This is what `arc_conv` does, and it uses Fate/Hollow Ataraxia's
   order as the initial permutation. This project follows the same approach.

   This means we need to translate given game's order of execution to the
   order we assumed during FHA decryption.

   To do that, we look at the `cxdec_key_deriver.rb` and compare what it does
   to what does the debugged CXDEC. In other words, we do as follows:

   1. Check which `JUMP` target starts with `push b8`. The `b8` is equivalent
      to what the code assembler puts into the buffer and is taken from
      `cx_key_deriver.rb` for the first switch jump.
   2. Check which `JUMP` starts with `push c7; push b8`.
   3. Figuring out the only remaining element is trivial.

   For example, if `push b8` is located at `JUMP2`, we start with `(?, 0, ?)`.
   Then if `push c7; push b8` is located at `JUMP1`, we continue with `(1, 0,
   ?)`. Then we finish with `(1, 0, 2)`.

   To find the second and third permutation, we search by analogy for a code
   that computes `x %= 6` and `x %= 8` (by looking for `mov register, 6; div
   register` and `mov register, 8; div register`). Then we figure out the
   switch block (i.e. `jz` and `jnz` jumps) just like we did above.

   Generally, CXDEC uses `push` statements to put stuff into the code buffer.
   This is what `Crass` presented well in its code: these `push`es are in
   fact arguments to functions like `push_5_bytes(b1, b2, b3, b4, b5)`.

This tutorial should be sufficient to create plugins for new games.

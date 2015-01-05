require 'lib/common'

# Decryption routine for Chaos;Head
class ChaosHeadFilter
  def permutation
    hex_s_to_a(
      'F1 71 80 19 17 01 74 7D 90 47 F9 68 DE B4 24 40' \
      '73 9E 5B 38 4C 3A 2A 0D 2E B9 5C E9 CE E8 3E 39' \
      'A2 F8 A8 5E 1D 1B D3 23 CB 9B B0 D5 59 F0 3B 09' \
      '4D E4 4A 30 7F 89 44 A0 7A 3C EE 0E 66 BF C9 46' \
      '77 21 86 78 6E 8E E6 99 33 2B 0C EA 42 85 D2 8F' \
      '5F 94 DA AC 76 B7 51 BA 0B D4 91 28 72 AE E7 D6' \
      'BD 53 A3 4F 9D C5 CC 5D 18 96 02 A5 C2 63 F4 00' \
      '6B EB 79 95 83 A7 8C 9A AB 8A 4E D7 DB CA 62 27' \
      '0A D1 DD 48 C6 88 B6 A9 41 10 FE 55 E0 D9 06 29' \
      '65 6A ED E5 98 52 FF 8D 43 F6 A4 CF A6 F2 97 13' \
      '12 04 FD 25 81 87 EF 2F 6C 84 2C AA A1 AF 36 CD' \
      '92 0F 2D 67 45 E2 64 B3 20 50 4B F3 7B 1F 1C 03' \
      'C4 C1 16 61 6F C7 BE 05 AD 22 34 B2 54 37 F7 D0' \
      'FA 60 8B 14 08 BC EC BB 26 9C 57 32 5A 3F 35 6D' \
      'C8 C3 69 7C 31 58 E3 75 D8 E1 C0 9F 11 B5 93 56' \
      'F5 1E B1 1A 70 3D FB 82 DC DF 7E 07 15 49 FC B8')
  end

  def file_name_key(key1, key2)
    key1 * key2
  end

  def data_key
    0x8765_4321
  end
end

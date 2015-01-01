require 'lib/formats/arc/pak2_nitroplus_archive'
require 'test/test_helper'

# Unit tests for Pak2NitroplusArchive
class Pak2NitroplusArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(Pak2NitroplusArchive)
  end

  def test_zlib
    TestHelper.generic_pack_and_unpack_test(
      Pak2NitroplusArchive,
      compressed: true)
  end

  def test_sjis
    TestHelper.generic_sjis_names_test(Pak2NitroplusArchive)
  end

  def test_backslash
    TestHelper.generic_backslash_test(Pak2NitroplusArchive)
  end
end

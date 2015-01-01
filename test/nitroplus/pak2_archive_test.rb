require_relative '../../lib/nitroplus/pak2_archive'
require_relative '../test_helper'

# Unit tests for Pak2Archive
class Pak2ArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(Pak2Archive)
  end

  def test_zlib
    TestHelper.generic_pack_and_unpack_test(Pak2Archive, compressed: true)
  end

  def test_sjis
    TestHelper.generic_sjis_names_test(Pak2Archive)
  end

  def test_backslash
    TestHelper.generic_backslash_test(Pak2Archive)
  end
end

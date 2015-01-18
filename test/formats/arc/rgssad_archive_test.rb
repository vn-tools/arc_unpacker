require 'lib/formats/arc/rgssad_archive'
require 'test/test_helper'

# Unit tests for RgssadArchive
class RgssadArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(RgssadArchive)
  end

  def test_backslash
    TestHelper.generic_backslash_test(RgssadArchive)
  end
end

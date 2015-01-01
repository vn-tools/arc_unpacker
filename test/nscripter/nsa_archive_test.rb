require_relative '../../lib/nscripter/nsa_archive'
require_relative '../test_helper'

# Unit tests for NsaArchive
class NsaArchiveTest < Test::Unit::TestCase
  def test_no_compression
    TestHelper.generic_pack_and_unpack_test(
      NsaArchive,
      compression: NsaArchive::NO_COMPRESSION)
  end

  def test_lzss_compression
    TestHelper.generic_pack_and_unpack_test(
      NsaArchive,
      compression: NsaArchive::LZSS_COMPRESSION)
  end

  def test_backslash
    TestHelper.generic_backslash_test(NsaArchive)
  end
end

require_relative '../../lib/nscripter/nsa_archive'
require_relative '../test_helper'

# Unit tests for NsaArchive
class NsaArchiveTest < Test::Unit::TestCase
  def test_no_compression
    arc = NsaArchive.new
    TestHelper.write_and_read(
      arc,
      compression: NsaArchive::NO_COMPRESSION)
    assert_equal('dir\\test.txt', arc.files[1].file_name)
  end

  def test_lzss_compression
    TestHelper.write_and_read(
      NsaArchive.new,
      compression: NsaArchive::LZSS_COMPRESSION)
  end
end

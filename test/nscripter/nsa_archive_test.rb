require_relative '../../lib/nscripter/nsa_archive'
require_relative '../test_helper'

# Unit tests for NsaArchive
class NsaArchiveTest < Test::Unit::TestCase
  def test_no_compression
    TestHelper.generic_pack_and_unpack_test(
      NsaArchive.new,
      compression: NsaArchive::NO_COMPRESSION)
  end

  def test_lzss_compression
    TestHelper.generic_pack_and_unpack_test(
      NsaArchive.new,
      compression: NsaArchive::LZSS_COMPRESSION)
  end

  def test_backslash
    input_files = InputFilesMock.new([
      { file_name: 'dir/test.txt', data: 'whatever' }])

    output_files = TestHelper.pack_and_unpack(NsaArchive.new, input_files)
    assert_equal('dir\\test.txt', output_files.files.first[:file_name])
  end
end

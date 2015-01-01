require_relative '../../lib/nitroplus/pak2_archive'
require_relative '../test_helper'

# Unit tests for Pak2Archive
class Pak2ArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(
      Pak2Archive::Packer.new,
      Pak2Archive::Unpacker.new)
  end

  def test_zlib
    TestHelper.generic_pack_and_unpack_test(
      Pak2Archive::Packer.new,
      Pak2Archive::Unpacker.new,
      compressed: true)
  end

  def test_sjis
    TestHelper.generic_sjis_names_test(
      Pak2Archive::Packer.new,
      Pak2Archive::Unpacker.new)
  end

  def test_backslash
    input_files = InputFilesMock.new([
      { file_name: 'dir/test.txt', data: 'whatever' }])

    output_files = TestHelper.pack_and_unpack(
      Pak2Archive::Packer.new,
      Pak2Archive::Unpacker.new,
      input_files)

    assert_equal('dir\\test.txt', output_files.files.first[:file_name])
  end
end

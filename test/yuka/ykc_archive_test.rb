require_relative '../../lib/yuka/ykc_archive'
require_relative '../test_helper'

# Unit tests for YkcArchive
class YkcArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(
      YkcArchive::Packer.new,
      YkcArchive::Unpacker.new)
  end

  def test_backslash
    input_files = InputFilesMock.new([
      { file_name: 'dir/test.txt', data: 'whatever' }])

    output_files = TestHelper.pack_and_unpack(
      YkcArchive::Packer.new,
      YkcArchive::Unpacker.new,
      input_files)

    assert_equal('dir\\test.txt', output_files.files.first[:file_name])
  end
end

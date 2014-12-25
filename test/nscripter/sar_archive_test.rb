require_relative '../../lib/nscripter/sar_archive'
require_relative '../test_helper'

# Unit tests for SarArchive
class SarArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(SarArchive.new)
  end

  def test_backslash
    input_files = InputFilesMock.new([
      {file_name: 'dir/test.txt', data: 'whatever'}])

    output_files = TestHelper.pack_and_unpack(SarArchive.new, input_files)
    assert_equal('dir\\test.txt', output_files.files.first[:file_name])
  end
end

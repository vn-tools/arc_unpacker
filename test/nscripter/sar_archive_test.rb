require_relative '../../lib/nscripter/sar_archive'
require_relative '../test_helper'

# Unit tests for SarArchive
class SarArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(SarArchive)
  end

  def test_backslash
    TestHelper.generic_backslash_test(SarArchive)
  end

  def test_file_order
    input_files = InputFilesMock.new([
      { file_name: '1.txt', data: 'whatever' },
      { file_name: '2.txt', data: 'whatever' }])

    output_files = TestHelper.pack_and_unpack(SarArchive, input_files)

    assert_equal('2.txt', output_files.files[0][:file_name])
    assert_equal('1.txt', output_files.files[1][:file_name])
  end
end

require 'lib/formats/arc/sar_archive'
require 'test/test_helper'

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
      MemoryFile.new('1.txt', 'whatever'),
      MemoryFile.new('2.txt', 'whatever')])

    output_files = TestHelper.pack_and_unpack(SarArchive, input_files)

    assert_equal('2.txt', output_files.files[0].name)
    assert_equal('1.txt', output_files.files[1].name)
  end
end

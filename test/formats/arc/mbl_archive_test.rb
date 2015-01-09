require 'lib/formats/arc/mbl_archive'
require 'test/test_helper'

# Unit tests for MblArchive
class MblArchiveTest < Test::Unit::TestCase
  def test_version2
    TestHelper.generic_pack_and_unpack_test(MblArchive, mbl_version: 2)
  end

  def test_sjis
    TestHelper.generic_sjis_names_test(MblArchive, mbl_version: 2)
  end

  def test_version1
    input_files = [{ file_name: 'short', data: 'whatever' }]

    output_files = TestHelper.pack_and_unpack(
      MblArchive,
      InputFilesMock.new(input_files),
      mbl_version: 1).files

    assert_equal(output_files, input_files)
  end

  def test_version1_too_long_names
    assert_raise(ArcError) do
      TestHelper.pack_and_unpack(
        MblArchive,
        InputFilesMock.new([{ file_name: 'long' * 10, data: 'whatever' }]),
        mbl_version: 1)
    end
  end
end

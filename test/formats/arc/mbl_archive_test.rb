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
    input_files = InputFilesMock.new([VirtualFile.new('short', 'whatever')])

    output_files = TestHelper.pack_and_unpack(
      MblArchive,
      input_files,
      mbl_version: 1)

    TestHelper.compare_files(input_files, output_files)
  end

  def test_version1_too_long_names
    input_files = InputFilesMock.new([VirtualFile.new('long' * 10, 'whatever')])

    assert_raise(RuntimeError) do
      TestHelper.pack_and_unpack(
        MblArchive,
        input_files,
        mbl_version: 1)
    end
  end
end

require 'lib/formats/arc/arc_archive'
require 'test/test_helper'

# Unit tests for ArcArchive
class ArcArchiveTest < Test::Unit::TestCase
  def test
    input_files = InputFilesMock.new([
      VirtualFile.new('1.txt', ''),
      VirtualFile.new('2.txt', TestHelper.rand_binary_string(3000))])

    output_files = TestHelper.pack_and_unpack(ArcArchive, input_files)

    TestHelper.compare_files(input_files, output_files)
  end
end

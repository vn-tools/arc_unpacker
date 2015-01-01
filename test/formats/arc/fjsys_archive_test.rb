require 'lib/formats/arc/fjsys_archive'
require 'test/test_helper'

# Unit tests for FjsysArchive
class FjsysArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(FjsysArchive)
  end

  def test_file_order
    ['cg_p.png', 'cg00.png', 'cg.png'].permutation.each do |file_names|
      files = file_names.map { |fn| { file_name: fn, data: 'whatever' } }
      input_files = InputFilesMock.new(files)

      output_files = TestHelper.pack_and_unpack(FjsysArchive, input_files)

      assert_equal('cg.png', output_files.files[0][:file_name])
      assert_equal('cg_p.png', output_files.files[1][:file_name])
      assert_equal('cg00.png', output_files.files[2][:file_name])
    end
  end
end

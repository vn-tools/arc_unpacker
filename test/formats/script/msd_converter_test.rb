require 'lib/formats/script/msd_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for MsdConverter
class MsdConverterTest < Test::Unit::TestCase
  def test_decoding
    input_data = TestHelper.get_test_file('msd/CGLIST.MSD')
    expected_data = TestHelper.get_test_file('msd/CGLIST-out.MSD')

    file = VirtualFile.new(nil, input_data)
    MsdConverter.decode!(file, msd_key: MsdConverter::COMMON_KEYS[:sonohana11])
    actual_data = file.data

    assert_equal(expected_data, actual_data)
  end

  def test_decoding_bad_key
    input_data = TestHelper.get_test_file('msd/CGLIST.MSD')
    file = VirtualFile.new(nil, input_data)
    assert_raise(RuntimeError) { MsdConverter.decode!(file, msd_key: 'bad') }
  end
end

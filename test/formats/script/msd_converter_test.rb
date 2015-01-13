require 'lib/formats/script/msd_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for MsdConverter
class MsdConverterTest < Test::Unit::TestCase
  def test_decoding
    encoded_data = TestHelper.get_test_file('CGLIST.MSD')
    decoded_data = TestHelper.get_test_file('CGLIST-out.MSD')
    file = VirtualFile.new(nil, encoded_data)
    MsdConverter.decode!(file, msd_key: MsdConverter::COMMON_KEYS[:sonohana11])
    actual_data = file.data

    assert_equal(decoded_data, actual_data)
  end

  def test_decoding_bad_key
    encoded_data = TestHelper.get_test_file('CGLIST.MSD')
    file = VirtualFile.new(nil, encoded_data)
    assert_raise(RuntimeError) { MsdConverter.decode!(file, msd_key: 'bad') }
  end
end

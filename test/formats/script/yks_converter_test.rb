require 'lib/formats/script/yks_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for YksConverter
class YksConverterTest < Test::Unit::TestCase
  def test_decoding_and_encoding_unencrypted
    input_data = TestHelper.get_test_file('yks/SelectJumpStart.yks')

    file = VirtualFile.new(nil, input_data)
    YksConverter.decode!(file, {})
    decoded_data = file.data
    YksConverter.encode!(file, {})
    YksConverter.decode!(file, {})
    decoded_again_data = file.data

    assert_equal(decoded_data, decoded_again_data)
  end

  def test_decoding_and_encoding_encrypted
    input_data = TestHelper.get_test_file('yks/SelectJumpStart.yks')

    file = VirtualFile.new(nil, input_data)
    YksConverter.decode!(file, {})
    decoded_data = file.data
    YksConverter.encode!(file, encrypt_yks: true)
    encoded_data = file.data
    YksConverter.decode!(file, {})
    decoded_again_data = file.data

    assert_equal(decoded_data, decoded_again_data)
    assert_equal(encoded_data, input_data)
  end
end

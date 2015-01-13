require 'lib/formats/script/yks_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for YksConverter
class YksConverterTest < Test::Unit::TestCase
  def test_decoding_and_encoding_unencrypted
    original = TestHelper.get_test_file('yks/SelectJumpStart.yks')
    file = VirtualFile.new(nil, original)
    YksConverter.decode!(file, {})
    decoded = file.data
    YksConverter.encode!(file, {})
    YksConverter.decode!(file, {})
    decoded_again = file.data
    assert_equal(decoded, decoded_again)
  end

  def test_decoding_and_encoding_encrypted
    original = TestHelper.get_test_file('yks/SelectJumpStart.yks')
    file = VirtualFile.new(nil, original)
    YksConverter.decode!(file, {})
    decoded = file.data
    YksConverter.encode!(file, encrypt_yks: true)
    encoded = file.data
    YksConverter.decode!(file, {})
    decoded_again = file.data
    assert_equal(decoded, decoded_again)
    assert_equal(encoded, original)
  end
end

require 'lib/formats/gfx/sotes_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for SotesConverter
class SotesConverterTest < Test::Unit::TestCase
  def test_decoding_palette
    input_data = TestHelper.get_test_file('sotes/#1726')
    expected_data = TestHelper.get_test_file('sotes/#1726-out.png')

    file = VirtualFile.new(nil, input_data)
    SotesConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end

  def test_decoding_non_palette
    input_data = TestHelper.get_test_file('sotes/#1410')
    expected_data = TestHelper.get_test_file('sotes/#1410-out.png')

    file = VirtualFile.new(nil, input_data)
    SotesConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end
end

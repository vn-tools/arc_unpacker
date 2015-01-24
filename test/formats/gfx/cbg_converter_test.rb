require 'lib/formats/gfx/cbg_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for CbgConverter
class CbgConverterTest < Test::Unit::TestCase
  def test_decoding_bgr
    input_data = TestHelper.get_test_file('cbg/3')
    expected_data = TestHelper.get_test_file('cbg/3-out.png')

    file = VirtualFile.new(nil, input_data)
    CbgConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end

  def test_decoding_grayscale
    input_data = TestHelper.get_test_file('cbg/4')
    expected_data = TestHelper.get_test_file('cbg/4-out.png')

    file = VirtualFile.new(nil, input_data)
    CbgConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end

  def test_decoding_bgr
    input_data = TestHelper.get_test_file('cbg/ti_si_de_a1')
    expected_data = TestHelper.get_test_file('cbg/ti_si_de_a1-out.png')

    file = VirtualFile.new(nil, input_data)
    CbgConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end
end

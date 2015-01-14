require 'lib/formats/gfx/spb_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for SpbConverter
class SpbConverterTest < Test::Unit::TestCase
  def test_decoding
    input_data = TestHelper.get_test_file('spb/grimoire_btn.bmp')
    expected_data = TestHelper.get_test_file('spb/grimoire_btn-out.png')

    file = VirtualFile.new(nil, input_data)
    SpbConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end
end

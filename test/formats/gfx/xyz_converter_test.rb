require 'lib/formats/gfx/xyz_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for XyzConverter
class XyzConverterTest < Test::Unit::TestCase
  def test_decoding
    input_data = TestHelper.get_test_file('xyz/浅瀬部屋a.xyz')
    expected_data = TestHelper.get_test_file('xyz/浅瀬部屋a-out.png')

    file = VirtualFile.new(nil, input_data)
    XyzConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end
end

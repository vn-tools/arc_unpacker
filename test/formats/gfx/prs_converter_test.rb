require 'lib/formats/gfx/prs_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for PrsConverter
class PrsConverterTest < Test::Unit::TestCase
  def test_decoding
    input_data = TestHelper.get_test_file('prs/BMIK_A16')
    expected_data = TestHelper.get_test_file('prs/BMIK_A16-out.png')

    file = VirtualFile.new(nil, input_data)
    PrsConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end
end

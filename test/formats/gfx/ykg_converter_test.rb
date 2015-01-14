require 'lib/formats/gfx/ykg_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for YkgConverter
class YkgConverterTest < Test::Unit::TestCase
  def test_decoding
    input_data = TestHelper.get_test_file('ykg/reimu.ykg')
    expected_data = TestHelper.get_test_file('ykg/reimu-out.png')

    file = VirtualFile.new(nil, input_data)
    YkgConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end

  def test_encoding_and_decoding
    test_regions = [
      { x: 0, y: 0, width: 641, height: 720 },
      { x: 330, y: 431, width: 123, height: 234 }]

    data = TestHelper.get_test_file('reimu_transparent.png')
    data = Image.add_meta_to_boxed(data, regions: test_regions)
    file = VirtualFile.new(nil, data)
    YkgConverter.encode!(file, {})
    YkgConverter.decode!(file, {})
    data = file.data
    regions = Image.read_meta_from_boxed(data)[:regions]

    assert_equal('PNG', data[1..3])
    assert_equal('IEND', data[-8..-5])
    assert_equal(test_regions, regions)
  end
end

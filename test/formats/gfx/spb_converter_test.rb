require 'lib/formats/gfx/spb_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for SpbConverter
class SpbConverterTest < Test::Unit::TestCase
  def test_decoding
    data = TestHelper.get_test_file('spb/grimoire_btn.bmp')
    file = VirtualFile.new(nil, data)
    SpbConverter.decode!(file, {})
    data = file.data

    assert_equal('PNG', data[1..3])
    assert_equal('IEND', data[-8..-5])
  end
end

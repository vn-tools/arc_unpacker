require 'lib/formats/gfx/prs_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for PrsConverter
class PrsConverterTest < Test::Unit::TestCase
  def test_decoding
    data = TestHelper.get_test_file('BMIK_A16')
    file = VirtualFile.new(nil, data)
    PrsConverter.decode!(file, {})
    data = file.data

    assert_equal('PNG', data[1..3])
    assert_equal('IEND', data[-8..-5])
  end
end

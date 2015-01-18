require 'lib/formats/gfx/tlg_converter'
require 'lib/virtual_file'
require 'test/test_helper'

# Unit tests for TlgConverter
class TlgConverterTest < Test::Unit::TestCase
  def test_decoding_version5
    input_data = TestHelper.get_test_file('tlg/14凛ペンダント.tlg')
    expected_data = TestHelper.get_test_file('tlg/14凛ペンダント-out.png')

    file = VirtualFile.new(nil, input_data)
    TlgConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end
end

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

  def test_decoding_version6
    input_data = TestHelper.get_test_file('tlg/tlg6.tlg')
    expected_data = TestHelper.get_test_file('tlg/tlg6-out.png')

    file = VirtualFile.new(nil, input_data)
    TlgConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
  end

  def test_decoding_version0
    input_data = TestHelper.get_test_file('tlg/bg08d.tlg')
    expected_data = TestHelper.get_test_file('tlg/bg08d-out.png')
    expected_meta = { tags: { mode: 'alpha' } }

    file = VirtualFile.new(nil, input_data)
    TlgConverter.decode!(file, {})
    actual_data = file.data
    actual_meta = Image.from_boxed(actual_data, nil).meta

    assert_equal(expected_meta, actual_meta)
    TestHelper.compare_image(expected_data, actual_data)
  end
end

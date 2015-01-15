require 'lib/formats/gfx/mgd_converter'
require 'lib/common'
require 'lib/virtual_file'
require 'lib/image'
require 'test/test_helper'
silence_warnings { require 'rmagick' }

# Unit tests for MgdConverter
class MgdConverterTest < Test::Unit::TestCase
  def test_decoding_from_sgd
    input_data = TestHelper.get_test_file('mgd/GS_UD.MGD')
    expected_data = TestHelper.get_test_file('mgd/GS_UD-out.png')

    file = VirtualFile.new(nil, input_data)
    MgdConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
    regions = Image.from_boxed(actual_data, nil).meta[:regions]
    assert_equal([{ width: 800, height: 600, x: 0, y: 0 }], regions)
  end

  def test_decoding_from_png
    input_data = TestHelper.get_test_file('mgd/saveload_p.MGD')
    expected_data = TestHelper.get_test_file('mgd/saveload_p-out.png')

    file = VirtualFile.new(nil, input_data)
    MgdConverter.decode!(file, {})
    actual_data = file.data

    TestHelper.compare_image(expected_data, actual_data)
    regions = Image.from_boxed(actual_data, nil).meta[:regions]
    assert_equal(
      [
        { width: 30, height: 30, x: 0, y: 300 },
        { width: 30, height: 30, x: 0, y: 330 },
        { width: 30, height: 30, x: 0, y: 360 },
        { width: 30, height: 30, x: 0, y: 390 },
        { width: 30, height: 30, x: 0, y: 420 },
        { width: 30, height: 30, x: 30, y: 300 },
        { width: 30, height: 30, x: 30, y: 330 },
        { width: 30, height: 30, x: 30, y: 360 },
        { width: 30, height: 30, x: 30, y: 390 },
        { width: 30, height: 30, x: 30, y: 420 },
        { width: 30, height: 30, x: 60, y: 300 },
        { width: 30, height: 30, x: 60, y: 330 },
        { width: 30, height: 30, x: 60, y: 360 },
        { width: 30, height: 30, x: 60, y: 390 },
        { width: 30, height: 30, x: 60, y: 420 },
        { width: 30, height: 30, x: 90, y: 300 },
        { width: 30, height: 30, x: 90, y: 330 },
        { width: 30, height: 30, x: 90, y: 360 },
        { width: 30, height: 30, x: 90, y: 390 },
        { width: 30, height: 30, x: 90, y: 420 },
        { width: 30, height: 30, x: 120, y: 300 },
        { width: 30, height: 30, x: 120, y: 330 },
        { width: 30, height: 30, x: 120, y: 360 },
        { width: 30, height: 30, x: 120, y: 390 },
        { width: 30, height: 30, x: 120, y: 420 },
        { width: 370, height: 60, x: 0, y: 0 },
        { width: 370, height: 60, x: 0, y: 60 },
        { width: 370, height: 60, x: 0, y: 120 },
        { width: 370, height: 60, x: 0, y: 180 },
        { width: 370, height: 60, x: 0, y: 240 },
        { width: 120, height: 30, x: 370, y: 0 },
        { width: 120, height: 30, x: 370, y: 30 },
        { width: 120, height: 30, x: 370, y: 60 },
        { width: 120, height: 30, x: 370, y: 90 },
        { width: 120, height: 30, x: 370, y: 120 }
      ],
      regions)
  end

  def test_encoding_and_decoding
    data = TestHelper.get_test_file('reimu_transparent.png')
    file = VirtualFile.new(nil, data)
    MgdConverter.encode!(file, {})
    MgdConverter.decode!(file, {})
    data = file.data
    regions = Image.from_boxed(data, nil).meta[:regions]

    image = Magick::Image.from_blob(data)[0]
    assert_equal(641, image.columns)
    assert_equal(720, image.rows)

    mul = 255.0 / Magick::QuantumRange
    assert_equal(254, image.pixel_color(200, 100).red * mul)
    assert_equal(10, image.pixel_color(200, 100).green * mul)
    assert_equal(23, image.pixel_color(200, 100).blue * mul)
    assert_equal(0, image.pixel_color(200, 100).opacity * mul)

    assert_equal([{ width: 641, height: 720, x: 0, y: 0 }], regions)
  end
end

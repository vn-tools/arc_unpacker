require 'lib/formats/gfx/mgd_converter'
require 'lib/common'
require 'lib/virtual_file'
require 'lib/image'
require 'test/test_helper'
silence_warnings { require 'rmagick' }

# Unit tests for MgdConverter
class MgdConverterTest < Test::Unit::TestCase
  def test_encoding_and_decoding
    data = TestHelper.get_test_file('reimu_transparent.png')
    file = VirtualFile.new(nil, data)
    MgdConverter.encode!(file, {})
    MgdConverter.decode!(file, {})
    data = file.data
    regions = Image.read_meta_from_boxed(data)[:regions]

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

  def test_decoding_from_sgd
    data = TestHelper.get_test_file('GS_UD.MGD')
    file = VirtualFile.new(nil, data)
    MgdConverter.decode!(file, {})
    data = file.data

    regions = Image.read_meta_from_boxed(data)[:regions]

    image = Magick::Image.from_blob(data)[0]
    assert_equal(800, image.columns)
    assert_equal(600, image.rows)

    assert_equal([{ width: 800, height: 600, x: 0, y: 0 }], regions)
  end

  def test_decoding_from_png
    data = TestHelper.get_test_file('saveload_p.MGD')
    file = VirtualFile.new(nil, data)
    MgdConverter.decode!(file, {})
    data = file.data

    regions = Image.read_meta_from_boxed(data)[:regions]

    image = Magick::Image.from_blob(data)[0]
    assert_equal(800, image.columns)
    assert_equal(600, image.rows)

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
end

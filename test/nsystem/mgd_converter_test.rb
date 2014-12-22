require_relative '../../lib/nsystem/mgd_converter'
require_relative '../test_helper'
require 'rmagick'

# Unit tests for MgdConverter
class MgdConverterTest < Test::Unit::TestCase
  def test_encoding_and_decoding
    data = TestHelper.get_test_file('reimu_transparent.png')
    data, regions = MgdConverter.decode(MgdConverter.encode(data))

    image = Magick::Image.from_blob(data)[0]
    assert_equal(641, image.columns)
    assert_equal(720, image.rows)

    assert_equal([{ width: 641, height: 720, x: 0, y: 0 }], regions)
  end

  def test_decoding_from_sgd
    data = TestHelper.get_test_file('GS_UD.MGD')
    data, regions = MgdConverter.decode(data)

    image = Magick::Image.from_blob(data)[0]
    assert_equal(800, image.columns)
    assert_equal(600, image.rows)

    assert_equal([{ width: 800, height: 600, x: 0, y: 0 }], regions)
  end

  def test_decoding_from_png
    data = TestHelper.get_test_file('saveload_p.MGD')
    data, regions = MgdConverter.decode(data)

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

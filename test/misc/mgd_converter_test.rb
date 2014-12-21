require_relative '../../lib/misc/mgd_converter'
require_relative '../test_helper'
require 'rmagick'

# Unit tests for MgdConverter
class MgdConverterTest < Test::Unit::TestCase
  def test_encoding_and_decoding
    data = TestHelper.get_test_file('reimu_transparent.png')
    data = MgdConverter.decode(MgdConverter.encode(data))
    image = Magick::Image.from_blob(data)[0]
    assert_equal(641, image.columns)
    assert_equal(720, image.rows)
  end

  def test_decoding_from_sgd
    data = TestHelper.get_test_file('GS_UD.MGD')
    data = MgdConverter.decode(data)
    image = Magick::Image.from_blob(data)[0]
    assert_equal(800, image.columns)
    assert_equal(600, image.rows)
  end

  def test_decoding_from_png
    data = TestHelper.get_test_file('saveload_p.MGD')
    data = MgdConverter.decode(data)
    image = Magick::Image.from_blob(data)[0]
    assert_equal(800, image.columns)
    assert_equal(600, image.rows)
  end
end

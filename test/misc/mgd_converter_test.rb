require_relative '../../lib/misc/fjsys_archive'
require_relative '../test_helper'
require 'rmagick'

# Unit tests for FjsysArchive
class MgdConverterTest < Test::Unit::TestCase
  def test
    data = TestHelper.get_test_file('reimu_transparent.png')
    data = MgdConverter.decode(MgdConverter.encode(data))
    image = Magick::Image.from_blob(data)[0]
    assert_equal(641, image.columns)
    assert_equal(720, image.rows)
  end
end

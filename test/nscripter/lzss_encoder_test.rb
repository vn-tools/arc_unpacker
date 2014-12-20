require_relative '../../lib/nscripter/lzss_encoder'
require 'test/unit'

# Unit tests for LzssEncoder
class LzssEncoderTest < Test::Unit::TestCase
  def test_empty
    test = ''
    lzss = LzssEncoder.new
    assert_equal(test, lzss.decode(lzss.encode(test)))
  end

  def test_one_letter
    test = 'a'
    lzss = LzssEncoder.new
    assert_equal(test, lzss.decode(lzss.encode(test)))
  end

  def test_long_without_repetitions
    test = 'abcdefghijklmnopqrstuvwxyz'
    lzss = LzssEncoder.new
    assert_equal(test, lzss.decode(lzss.encode(test)))
  end

  def test_repetitions
    (0..1000).step(7).each do |i|
      test = 's' * i
      lzss = LzssEncoder.new
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions
    (0..100).step(7).each do |i|
      test = 'maslo ' * i
      lzss = LzssEncoder.new
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_custom_settings
    (0..100).step(7).each do |i|
      test = 'maslo ' * i
      lzss = LzssEncoder.new(position_bits: 5, length_bits: 3)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end
end

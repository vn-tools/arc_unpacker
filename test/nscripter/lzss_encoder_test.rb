require_relative '../../lib/nscripter/lzss_encoder'
require_relative '../test_helper'
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
    test = ''
    (0..1000).step(7).each do
      test += '#' * 5
      lzss = LzssEncoder.new
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions
    (0..300).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssEncoder.new
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_custom_size
    (0..100).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssEncoder.new(position_bits: 5, length_bits: 3)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_non_zero_dictionary_pos
    (0..100).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssEncoder.new(initial_dictionary_pos: 15)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_reuse_compressed
    (0..100).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssEncoder.new(reuse_compressed: true)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end
end

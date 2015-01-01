require 'lib/formats/arc/nsa_archive/lzss_compressor'
require 'test/test_helper'
require 'test/unit'

# Unit tests for LzssCompressor
class LzssCompressorTest < Test::Unit::TestCase
  def test_empty
    test = ''
    lzss = LzssCompressor.new
    assert_equal(test, lzss.decode(lzss.encode(test)))
  end

  def test_one_letter
    test = 'a'
    lzss = LzssCompressor.new
    assert_equal(test, lzss.decode(lzss.encode(test)))
  end

  def test_long_without_repetitions
    test = 'abcdefghijklmnopqrstuvwxyz'
    lzss = LzssCompressor.new
    assert_equal(test, lzss.decode(lzss.encode(test)))
  end

  def test_repetitions
    test = ''
    (0..1000).step(7).each do
      test += '#' * 5
      lzss = LzssCompressor.new
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions
    (0..300).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssCompressor.new
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_custom_size
    (0..100).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssCompressor.new(position_bits: 5, length_bits: 3)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_non_zero_dictionary_pos
    (0..100).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssCompressor.new(initial_dictionary_pos: 15)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end

  def test_complex_repetitions_reuse_compressed
    (0..100).each do |i|
      test = TestHelper.rand_string(i)
      lzss = LzssCompressor.new(reuse_compressed: true)
      assert_equal(test, lzss.decode(lzss.encode(test)))
    end
  end
end

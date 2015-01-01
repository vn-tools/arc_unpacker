require 'test/unit'
require 'lib/bit_stream'

# Unit tests for BitStream
class BitStreamTest < Test::Unit::TestCase
  def test_tell
    bs = BitStream.new("\xff")
    assert_equal(0, bs.tell)
    bs.read(1)
    assert_equal(1, bs.tell)
  end

  def test_seek
    bs = BitStream.new("\xc0")
    bs.seek(1)
    assert_equal(1, bs.tell)
    assert_equal(2, bs.read(2))
  end

  def test_seek_absolute_and_relative
    bs = BitStream.new("\xc0")
    bs.seek(1)
    bs.seek(1, BitStream::SEEK_SET)
    assert_equal(1, bs.tell)
    bs.seek(1, BitStream::SEEK_CUR)
    assert_equal(2, bs.tell)
  end

  def test_seek_past_eof_and_back
    bs = BitStream.new("\xc0")
    bs.seek(100)
    bs.seek(0, BitStream::SEEK_SET)
    assert_equal(3, bs.read(2))
    assert_equal(2, bs.tell)
  end

  def test_eof
    bs = BitStream.new("\xc0")
    bs.seek(1)
    assert_equal(false, bs.eof?)
    bs.seek(7)
    assert_equal(true, bs.eof?)
    bs.seek(6, BitStream::SEEK_SET)
    assert_equal(false, bs.eof?)
    bs.read(1)
    assert_equal(false, bs.eof?)
    bs.read(1)
    assert_equal(true, bs.eof?)
  end

  def test_read_at_start
    (0..16).each do |i|
      ones = (1 << i) - 1
      bs = BitStream.new("\xff\xff")
      assert_equal(ones, bs.read(i))
    end
  end

  def test_read_at_offset
    (0..16).each do |i|
      ones = (1 << i) - 1
      bs = BitStream.new("\x7f\xff\x80")
      assert_equal(0, bs.read(1))
      bs.seek(1, BitStream::SEEK_SET)
      assert_equal(ones, bs.read(i))
    end
  end

  def test_boundary
    bs = BitStream.new("\x01\x80")
    assert_equal(0, bs.read(7))
    assert_equal(3, bs.read(2))
    assert_equal(0, bs.read(7))
  end

  def test_read_after_eof
    bs = BitStream.new("\xff")
    assert_nil(bs.read(9))
  end

  def test_endianness
    bs = BitStream.new("\xa5")
    assert_equal(1, bs.read(1))
    assert_equal(0, bs.read(1))
    assert_equal(1, bs.read(1))
    assert_equal(0, bs.read(1))
    assert_equal(0, bs.read(1))
    assert_equal(1, bs.read(1))
    assert_equal(0, bs.read(1))
    assert_equal(1, bs.read(1))
  end

  def test_write_at_start
    (0..16).each do |i|
      ones = (1 << i) - 1
      bs = BitStream.new
      bs.write(ones, i)
      assert_equal(i, bs.tell)
      bs.seek(0, BitStream::SEEK_SET)
      assert_equal(ones, bs.read(i), 'fail for ' + i.to_s)
    end
  end

  def test_write_at_offset
    (1..8).each do |offset|
      (0..16).each do |i|
        ones = (1 << i) - 1
        bs = BitStream.new("\x00")
        bs.seek(offset, BitStream::SEEK_SET)
        bs.write(ones, i)
        assert_equal(offset + i, bs.tell)
        bs.seek(offset, BitStream::SEEK_SET)
        assert_equal(ones, bs.read(i), 'fail for ' + i.to_s)
      end
    end
  end

  def test_overwrite
    bs = BitStream.new("\xff\xff")
    bs.seek(7)
    bs.write(0, 2)
    bs.seek(0, BitStream::SEEK_SET)
    assert_equal(254, bs.read(8))
    assert_equal(127, bs.read(8))
  end

  def test_bytes
    bs = BitStream.new("\x12\x34")
    bs.seek(7)
    assert_equal("\x12\x34", bs.bytes)
    assert_equal(7, bs.tell)
  end
end

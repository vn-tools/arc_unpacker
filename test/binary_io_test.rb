require 'test/unit'
require_relative '../lib/binary_io'

# Unit tests for BinaryIO
class BinaryIOTest < Test::Unit::TestCase
  def test_creating_from_string
    assert_nothing_thrown { BinaryIO.from_string('test') }
  end

  def test_creating_from_file
    assert_nothing_thrown { BinaryIO.from_file(__FILE__) }
  end

  def test_methods_on_file
    io = BinaryIO.from_file(__FILE__)
    assert_equal(true, io.respond_to?(:seek))
    assert_nothing_thrown { io.seek(0) }
  end

  def test_methods_on_string
    io = BinaryIO.from_string('test')
    assert_equal(true, io.respond_to?(:seek))
    assert_nothing_thrown { io.seek(0) }
  end

  def test_block_on_file
    ret = BinaryIO.from_file(__FILE__) do |io|
      assert_equal(true, io.respond_to?(:seek))
      assert_nothing_thrown { io.seek(0) }
      'done'
    end
    assert_equal('done', ret)
  end

  def test_block_on_string
    ret = BinaryIO.from_string('test') do |io|
      assert_equal(true, io.respond_to?(:seek))
      assert_nothing_thrown { io.seek(0) }
      'done'
    end
    assert_equal('done', ret)
  end
end

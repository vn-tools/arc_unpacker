require_relative '../../lib/renpy/pickle'
require 'test/unit'

# Unit tests for Unpickle
class PickleDecodingTest < Test::Unit::TestCase
  def test_consts
    do_test_object(true)
    do_test_object(false)
    do_test_object(nil)
  end

  def test_float
    do_test_object(0.4)
  end

  def test_integer
    do_test_object(0xff_ff_ff)
  end

  def test_long
    do_test_object(0xffff_ffff_ffff)
  end

  def test_string
    do_test_object('test')
  end

  def test_escaped_string
    do_test_object("test\r\n\t\\'\"A")
  end

  def test_unicode_string
    do_test_object('良い狩り')
  end

  def test_unicode_escaped_string
    do_test_object("test\r\n\t'\"\\")
  end

  def test_dictionary
    hash = {
      'str1' => 123,
      'str2' => 456,
      'str3' => 789
    }
    do_test_object(hash)
  end

  def test_tuple
    obj = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    obj.freeze
    do_test_object(obj)
  end

  def test_list
    do_test_object([1, 2, 3, 4, 5, 6, 7, 8, 9])
  end

  def test_nested_list
    do_test_object([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
  end

  def test_nested_dictionary
    do_test_object('a' => { 'b' => 'c' }, 'd' => { 'e' => 'f' })
  end

  def test_unsupported
    assert_raise(Pickle::EncoderError) { do_test_object(:symbol) }
  end

  private

  def do_test_object(obj)
    assert_equal(obj, Pickle.loads(Pickle.dumps(obj)))
  end
end

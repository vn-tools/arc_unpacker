require_relative '../../lib/renpy/pickle'
require 'test/unit'

# Unit tests for Unpickle
class PickleDecodingTest < Test::Unit::TestCase
  def test_consts
    assert_equal(true, Pickle.loads("\x88.\n"))
    assert_equal(false, Pickle.loads("\x89.\n"))
    assert_equal(nil, Pickle.loads("N.\n"))
  end

  def test_float
    assert_equal(0.4, Pickle.loads("F0.4\n.\n"))
  end

  def test_integer
    assert_equal(1, Pickle.loads("I1\n.\n"))
  end

  def test_long
    assert_equal(0xffff_ffff_ffff, Pickle.loads("L281474976710655L\n.\n"))
  end

  def test_string
    assert_equal('test', Pickle.loads("S'test'\np0\n.\n"))
  end

  def test_escaped_string
    assert_equal(
      "test\r\n\t\\'\"A",
      Pickle.loads("S\"test\\r\\n\\t\\\\\\'\\\"\\x41\"\np0\n.\n"))
  end

  def test_unicode_string
    assert_equal(
      '良い狩り',
      Pickle.loads("V\\u826f\\u3044\\u72e9\\u308a\np0\n.\n"))
  end

  def test_unicode_escaped_string
    assert_equal(
      "test\r\n\t'\"\\",
      Pickle.loads("Vtest\r\\u000a\t'\"\\u005c\np0\n.\n"))
  end

  def test_dictionary
    expected = {
      'str1' => 123,
      'str2' => 456,
      'str3' => 789
    }

    assert_equal(
      expected,
      Pickle.loads(
        "(dp0\nS'str3'\np1\nI789\nsS'str2'\n" \
        "p2\nI456\nsS'str1'\np3\nI123\ns.\n"))
  end

  def test_list
    actual = Pickle.loads(
      "(lp0\nI1\naI2\naI3\naI4\naI5\naI6\naI7\naI8\naI9\na.\n")
    assert_equal([1, 2, 3, 4, 5, 6, 7, 8, 9], actual)
    assert_equal(false, actual.frozen?)
  end

  def test_tuple
    actual = Pickle.loads("(I1\nI2\nI3\nI4\nI5\nI6\nI7\nI8\nI9\ntp0\n.\n")
    assert_equal([1, 2, 3, 4, 5, 6, 7, 8, 9], actual)
    assert_equal(true, actual.frozen?)
  end

  def test_nested_list
    assert_equal(
      [[1, 2, 3], [4, 5, 6], [7, 8, 9]],
      Pickle.loads(
        "(lp0\n(lp1\nI1\naI2\naI3\naa(lp2\nI4\n" \
        "aI5\naI6\naa(lp3\nI7\naI8\naI9\naa.\n"))
  end

  def test_nested_dictionary
    assert_equal(
      { 'a' => { 'b' => 'c' }, 'd' => { 'e' => 'f' } },
      Pickle.loads(
        "(dp0\nS'a'\np1\n(dp2\nS'b'\np3\nS'c'\np4\n" \
        "ssS'd'\np5\n(dp6\nS'e'\np7\nS'f'\np8\nss.\n"))
  end
end

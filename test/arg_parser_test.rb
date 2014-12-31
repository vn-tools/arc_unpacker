require 'test/unit'
require_relative '../lib/arg_parser'

# Unit tests for ArgParser
class ArgParserTest < Test::Unit::TestCase
  def test_instantiation
    assert_nothing_thrown { ArgParser.new([]) }
  end

  def test_switch_missing
    args = ArgParser.new([])
    assert_nil(args.switch(%w(-s --long)))
  end

  def test_switch_short
    args = ArgParser.new(%w(-s=short))
    assert_equal('short', args.switch(%w(-s --long)))
  end

  def test_switch_long
    args = ArgParser.new(%w(--long=long))
    assert_equal('long', args.switch(%w(-s --long)))
  end

  def test_switch_overriding_short
    args = ArgParser.new(%w(-s=short1 -s=short2))
    assert_equal('short2', args.switch(%w(-s --long)))
  end

  def test_switch_overriding_long
    args = ArgParser.new(%w(--long=long1 --long=long2))
    assert_equal('long2', args.switch(%w(-s --long)))
  end

  def test_switch_overriding_mixed
    args = ArgParser.new(%w(-s=short --long=long))
    assert_equal('short', args.switch(%w(-s --long)))
    assert_equal('long', args.switch(%w(--long -s)))
  end

  def test_switch_with_space
    args = ArgParser.new(['--switch=long switch'])
    assert_equal('long switch', args.switch('switch'))
  end

  def test_flag_missing
    args = ArgParser.new([])
    assert_equal(false, args.flag?('nope'))
  end

  def test_flag
    args = ArgParser.new(%w(--flag))
    assert_equal(true, args.flag?('flag'))
  end

  def test_flags_mixed_with_stray
    args = ArgParser.new(%w(--flag stray))
    assert_equal(true, args.flag?('flag'))
    assert_equal(%w(stray), args.stray)
  end

  def test_stray_missing
    args = ArgParser.new([])
    assert_equal([], args.stray)
  end

  def def_stray
    args = ArgParser.new(%w(stray1 stray2))
    assert_equal(%w(stray1 stray2), args.stray)
  end

  def test_stray_with_space
    args = ArgParser.new(['long stray'])
    assert_equal(['long stray'], args.stray)
  end

  def test_mixed_types
    args = ArgParser.new(%w(stray1 --switch=s --flag1 stray2 --flag2))

    assert_equal(%w(stray1 stray2), args.stray)
    assert_equal(true, args.flag?('flag1'))
    assert_equal(true, args.flag?('flag2'))
    assert_equal('s', args.switch('switch'))
  end
end

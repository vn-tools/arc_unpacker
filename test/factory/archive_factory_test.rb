require 'lib/arg_parser'
require 'lib/factory/archive_factory'
require 'test/unit'

class ArchiveFactoryTest < Test::Unit::TestCase
  def test_iteration
    assert_nothing_thrown { ArchiveFactory.each {} }
  end

  def test_cli_integration_help
    arg_parser = ArgParser.new([])
    options = {}
    ArchiveFactory.each do |_format, arc|
      assert_nothing_thrown { arc.add_cli_help(arg_parser) }
      assert_nothing_thrown { arc.parse_cli_options(arg_parser, options) }
    end
  end
end

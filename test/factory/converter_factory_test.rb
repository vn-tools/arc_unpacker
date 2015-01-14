require 'test/unit'
require 'lib/factory/converter_factory'

class ConverterFactoryTest < Test::Unit::TestCase
  def test_iteration
    assert_nothing_thrown { ConverterFactory.each {} }
  end

  def test_cli_integration_help
    arg_parser = ArgParser.new([])
    options = {}
    ConverterFactory.each do |_format, converter|
      assert_nothing_thrown { converter.add_cli_help(arg_parser) }
      assert_nothing_thrown { converter.parse_cli_options(arg_parser, options) }
    end
  end
end

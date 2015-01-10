require 'test/unit'
require 'lib/factory/converter_factory'

class ConverterFactoryTest < Test::Unit::TestCase
  def test_iteration
    assert_nothing_thrown { ConverterFactory.each {} }
  end
end

require 'test/unit'
require 'lib/archive_factory'

class ArchiveFactoryTest < Test::Unit::TestCase
  def test_iteration
    assert_nothing_thrown { ArchiveFactory.each {} }
  end
end

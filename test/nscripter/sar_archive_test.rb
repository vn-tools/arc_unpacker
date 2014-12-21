require_relative '../../lib/nscripter/sar_archive'
require_relative '../test_helper'

# Unit tests for SarArchive
class SarArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.write_and_read(SarArchive.new)
  end
end

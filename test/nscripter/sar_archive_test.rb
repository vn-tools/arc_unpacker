require_relative '../../lib/nscripter/sar_archive'
require_relative '../test_helper'

# Unit tests for SarArchive
class SarArchiveTest < Test::Unit::TestCase
  def test
    arc = SarArchive.new
    TestHelper.write_and_read(arc)
    assert_equal('dir\\test.txt', arc.files[1].file_name)
  end
end

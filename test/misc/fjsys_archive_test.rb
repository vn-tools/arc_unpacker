require_relative '../../lib/misc/fjsys_archive'
require_relative '../test_helper'

# Unit tests for FjsysArchive
class FjsysArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.write_and_read(FjsysArchive.new)
  end
end

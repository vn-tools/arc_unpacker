require_relative '../../lib/nsystem/fjsys_archive'
require_relative '../test_helper'

# Unit tests for FjsysArchive
class FjsysArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(FjsysArchive.new)
  end
end

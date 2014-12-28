require_relative '../../lib/ivory/mbl_archive'
require_relative '../test_helper'

# Unit tests for MblArchive
class MblArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(MblArchive.new)
  end
end

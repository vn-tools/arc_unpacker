require 'lib/formats/arc/npa_sg_archive'
require 'test/test_helper'

# Unit tests for NpaSgArchive
class NpaSgArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(NpaSgArchive)
  end
end

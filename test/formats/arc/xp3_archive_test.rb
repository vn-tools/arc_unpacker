require 'lib/formats/arc/xp3_archive'
require 'test/test_helper'

# Unit tests for Xp3Archive
class Xp3ArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(Xp3Archive, filter: :none)
  end

  def test_fsn
    TestHelper.generic_pack_and_unpack_test(Xp3Archive, filter: :fsn)
  end

  def test_fha
    TestHelper.generic_pack_and_unpack_test(Xp3Archive, filter: :fha)
  end

  def test_compressed_table
    TestHelper.generic_pack_and_unpack_test(
      Xp3Archive,
      filter: :none,
      compress_table: true)
  end

  def test_compressed_files
    TestHelper.generic_pack_and_unpack_test(
      Xp3Archive,
      filter: :none,
      compress_files: true)
  end
end

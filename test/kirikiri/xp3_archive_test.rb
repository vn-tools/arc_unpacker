require_relative '../../lib/kirikiri/xp3_archive'
require_relative '../test_helper'

# Unit tests for Xp3Archive
class Xp3ArchiveTest < Test::Unit::TestCase
  def test
    TestHelper.generic_pack_and_unpack_test(Xp3Archive, decryptor: :none)
  end

  def test_fsn
    TestHelper.generic_pack_and_unpack_test(Xp3Archive, decryptor: :fsn)
  end

  def test_fha
    TestHelper.generic_pack_and_unpack_test(Xp3Archive, decryptor: :fha)
  end

  def test_compressed_table
    TestHelper.generic_pack_and_unpack_test(
      Xp3Archive,
      decryptor: :none,
      compress_table: true)
  end

  def test_compressed_files
    TestHelper.generic_pack_and_unpack_test(
      Xp3Archive,
      decryptor: :none,
      compress_files: true)
  end
end

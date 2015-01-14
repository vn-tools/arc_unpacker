require 'lib/virtual_file'
require 'test/unit'

# Unit tests for BitStream
class VirtualFileTest < Test::Unit::TestCase
  def test_changing_extension_for_nil
    file = VirtualFile.new(nil, nil)
    file.change_extension('new')
    assert_nil(file.name)
  end

  def test_changing_extension
    file = VirtualFile.new('file.old', nil)
    file.change_extension('new')
    assert_equal('file.new', file.name)
  end

  def test_changing_extension_with_dot
    file = VirtualFile.new('file.old', nil)
    file.change_extension('.new')
    assert_equal('file.new', file.name)
  end

  def test_adding_extension
    file = VirtualFile.new('file', nil)
    file.change_extension('new')
    assert_equal('file.new', file.name)
  end

  def test_adding_extension_with_dot
    file = VirtualFile.new('file', nil)
    file.change_extension('.new')
    assert_equal('file.new', file.name)
  end
end

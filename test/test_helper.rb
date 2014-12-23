require 'test/unit'
require_relative '../lib/binary_io'
require_relative '../lib/file_entry'

include Test::Unit::Assertions

# Utility methods for the tests.
module TestHelper
  module_function

  def get_test_file(name)
    path = File.join(__dir__, 'test_files', name)
    open(path, 'rb') { |f| f.read }
  end

  def write_and_read(arc, options = {})
    buffer = BinaryIO.new
    content1 = rand_binary_string(30_000)
    content2 = rand_binary_string(1)
    arc.files.push(FileEntry.new('test.png', ->() { content1 }))
    arc.files.push(FileEntry.new('dir/test.txt', ->() { content2 }))
    arc.files.push(FileEntry.new('empty', ->() { '' }))
    arc.write_internal(buffer, options)

    buffer.seek(0, IO::SEEK_SET)
    arc.read_internal(buffer)
    assert_equal(3, arc.files.length)

    file1, = arc.files.select { |f| f.file_name == 'test.png' }
    file2, = arc.files.select { |f| f.file_name =~ /dir[\\\/]test.txt/ }
    file3, = arc.files.select { |f| f.file_name == 'empty' }

    assert_not_nil(file1)
    assert_not_nil(file2)
    assert_not_nil(file3)
    assert_equal(content1, file1.data.call)
    assert_equal(content2, file2.data.call)
    assert_equal('', file3.data.call)
  end

  def rand_string(length)
    (1..length).map { rand(2) == 0 ? '#' : '.' } * ''
  end

  def rand_binary_string(length)
    (1..length).map { rand(0xff).chr } * ''
  end
end

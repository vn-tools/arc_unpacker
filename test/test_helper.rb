require 'test/unit'
require_relative '../lib/binary_io'
require_relative 'output_files_mock'
require_relative 'input_files_mock'

include Test::Unit::Assertions

# Utility methods for the tests.
module TestHelper
  module_function

  def get_test_file(name)
    path = File.join(__dir__, 'test_files', name)
    open(path, 'rb') { |f| f.read }
  end

  def pack_and_unpack(arc, input_files, options = {})
    buffer = BinaryIO.new
    arc.pack_internal(buffer, input_files, options)

    buffer.rewind
    output_files = OutputFilesMock.new
    arc.unpack_internal(buffer, output_files)
    output_files
  end

  def generic_pack_and_unpack_test(arc, options = {})
    content1 = rand_binary_string(30_000)
    content2 = rand_binary_string(1)

    input_files = InputFilesMock.new([
      {file_name: 'test.png', data: content1},
      {file_name: 'dir/test.txt', data: content2},
      {file_name: 'empty', data: ''.b}],
      nil)
    output_files = pack_and_unpack(arc, input_files, options)

    files = output_files.files
    assert_equal(3, files.length)
    file1, = files.select { |f| f[:file_name] == 'test.png' }
    file2, = files.select { |f| f[:file_name] =~ /dir[\\\/]test.txt/ }
    file3, = files.select { |f| f[:file_name] == 'empty' }

    assert_not_nil(file1)
    assert_not_nil(file2)
    assert_not_nil(file3)
    assert_equal(content1, file1[:data])
    assert_equal(content2, file2[:data])
    assert_equal('', file3[:data])
  end

  def rand_string(length)
    (1..length).map { rand(2) == 0 ? '#' : '.' } * ''
  end

  def rand_binary_string(length)
    (1..length).map { rand(0xff).chr } * ''
  end
end

require 'test/unit'
require 'lib/common'
require 'lib/binary_io'
require 'test/output_files_mock'
require 'test/input_files_mock'

include Test::Unit::Assertions

# Utility methods for the tests.
module TestHelper
  module_function

  def get_test_file(name)
    path = File.join(__dir__, 'test_files', name)
    File.binread(path)
  end

  def pack_and_unpack(archive, input_files, options = {})
    buffer = BinaryIO.from_string('')
    archive::Packer.new.pack(buffer, input_files, options)
    buffer.rewind
    output_files = OutputFilesMock.new
    archive::Unpacker.new.unpack(buffer, output_files, options)
    output_files
  end

  def generic_backslash_test(archive, options = {})
    test_files = [VirtualFile.new('dir/test.txt', 'whatever')]
    input_files = InputFilesMock.new(test_files)
    output_files = pack_and_unpack(archive, input_files, options)
    assert_equal('dir\\test.txt', output_files.files.first.name)
  end

  def generic_sjis_names_test(archive, options = {})
    test_files = [VirtualFile.new('シフトジス', 'whatever')]
    input_files = InputFilesMock.new(test_files)
    output_files = pack_and_unpack(archive, input_files, options)
    compare_files(input_files, output_files)
  end

  def generic_pack_and_unpack_test(archive, options = {})
    test_files = []
    test_files << VirtualFile.new('empty.txt', ''.b)
    25.times do
      test_files << VirtualFile.new(
        rand_string(300),
        rand_binary_string(rand(1000)))
    end

    input_files = InputFilesMock.new(test_files)
    output_files = pack_and_unpack(archive, input_files, options)
    compare_files(input_files, output_files)
  end

  def compare_files(input_files, output_files)
    actual = output_files.files.sort_by(&:name)
    expected = input_files.files.sort_by(&:name)
    assert_equal(expected.map(&:name), actual.map(&:name))
    assert_equal(expected.map(&:data), actual.map(&:data))
  end

  def rand_string(length)
    (1..length).map { rand(2) == 0 ? '#' : '.' } * ''
  end

  def rand_binary_string(length)
    (1..length).map { rand(0xff).chr } * ''
  end
end

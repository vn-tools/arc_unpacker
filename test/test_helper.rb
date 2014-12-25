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
    File.binread(path)
  end

  def pack_and_unpack(arc, input_files, options = {})
    buffer = BinaryIO.from_string('')
    arc.pack_internal(buffer, input_files, options)

    buffer.rewind
    output_files = OutputFilesMock.new
    arc.unpack_internal(buffer, output_files)
    output_files
  end

  def generic_pack_and_unpack_test(arc, options = {})
    content1 = rand_binary_string(30_000)
    content2 = rand_binary_string(1)

    test_files = []
    test_files << {
      file_name: 'empty.txt',
      data: ''.b
    }
    25.times do
      test_files << {
        file_name: rand_string(300),
        data: rand_binary_string(rand(1000))
      }
    end

    test_files = test_files.sort_by { |f| f[:file_name] }

    input_files = InputFilesMock.new(test_files)
    output_files = pack_and_unpack(arc, input_files, options)

    files = output_files.files.sort_by { |f| f[:file_name] }
    assert_equal(files, test_files)
  end

  def rand_string(length)
    (1..length).map { rand(2) == 0 ? '#' : '.' } * ''
  end

  def rand_binary_string(length)
    (1..length).map { rand(0xff).chr } * ''
  end
end

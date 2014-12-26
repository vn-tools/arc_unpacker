require_relative 'binary_io'
require_relative 'input_files'
require_relative 'output_files'

# Generic archive
class Archive
  def unpack(source_arc, target_dir, options)
    BinaryIO.from_file(source_arc, 'rb') do |arc_file|
      unpack_internal(arc_file, OutputFiles.new(target_dir, options), options)
    end
  end

  def pack(source_dir, target_arc, options)
    BinaryIO.from_file(target_arc, 'wb') do |arc_file|
      pack_internal(arc_file, InputFiles.new(source_dir, options), options)
    end
  end

  protected

  def unpack_internal(_arc_file, _output_files, _options)
    fail 'This format does not support unpacking'
  end

  def pack_internal(_arc_file, _input_files, _options)
    fail 'This format does not support packing'
  end
end

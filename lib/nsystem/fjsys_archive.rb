require 'tmpdir'
require_relative '../archive'
require_relative '../file_entry'
require_relative 'mgd_converter'

# FJSYS archive
class FjsysArchive < Archive
  MAGIC = "FJSYS\x00\x00\x00"

  def read_internal(arc_file)
    magic = arc_file.read(MAGIC.length)
    fail 'Not a FJSYS archive' unless magic == MAGIC

    header_size,
    file_names_size,
    file_count = arc_file.read(76).unpack('LLLx64')
    file_names_start = header_size - file_names_size

    @files = (1..file_count).map do
      file_name_origin,
      data_size,
      data_origin = arc_file.read(16).unpack('LLQ')

      file_name = peek(arc_file, file_name_origin + file_names_start) do
        read_until_zero(arc_file)
      end

      data = lambda do
        arc_file.seek(data_origin, IO::SEEK_SET)
        decode(file_name, arc_file.read(data_size))
      end

      FileEntry.new(file_name, data)
    end
  end

  def write_internal(arc_file, _options)
    arc_file.write(MAGIC)

    file_names_size = @files.map { |f| f.file_name.length + 1 }.reduce(0, :+)
    file_names_start = @files.length * 16 + 0x54
    header_size = file_names_size + file_names_start
    arc_file.write([header_size, file_names_size, @files.length].pack('LLLx64'))
    arc_file.write("\x00" * (header_size - arc_file.tell))

    table_entries = []
    cur_data_origin = header_size
    cur_file_name_origin = 0
    @files.each do |file_entry|
      data = encode(file_entry.file_name, file_entry.data.call)
      data_size = data.length
      arc_file.write(data)

      table_entries.push([
        cur_file_name_origin,
        data_size,
        cur_data_origin])
      cur_file_name_origin += file_entry.file_name.length + 1
      cur_data_origin += data_size
      fail 'Bad' if arc_file.tell != cur_data_origin
    end

    arc_file.seek(0x54)
    table_entries.each do |file_name_origin, data_size, data_origin|
      arc_file.write([file_name_origin, data_size, data_origin].pack('LLQ'))
    end

    @files.each do |file_entry|
      arc_file.write(file_entry.file_name)
      arc_file.write("\x00".b)
    end
  end

  private

  def decode(file_name, data)
    if data[0..MgdConverter::MAGIC.length - 1] == MgdConverter::MAGIC
      data, regions = MgdConverter.decode(data)
      @meta = { regions: {} } if @meta.nil?
      @meta[:regions][file_name.to_sym] = regions
      return data
    end

    data
  end

  def encode(file_name, data)
    if file_name.downcase.end_with?('.mgd')
      regions = @meta[:regions][file_name.to_sym]
      return MgdConverter.encode(data, regions)
    end

    data
  end


  def peek(arc_file, pos, &func)
    old_pos = arc_file.tell
    arc_file.seek(pos)
    ret = func.call
    arc_file.seek(old_pos)
    ret
  end

  def read_until_zero(arc_file)
    str = ''
    while ((c = arc_file.read(1)) || "\0") != "\0"
      str += c
    end
    str
  end
end

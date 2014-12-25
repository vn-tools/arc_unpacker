require_relative '../archive'
require_relative 'ykg_converter'

# YKC archive
class YkcArchive < Archive
  MAGIC = 'YKC001'

  def unpack_internal(arc_file, output_files)
    magic = arc_file.read(MAGIC.length)
    fail 'Not a YKC archive' unless magic == MAGIC

    _version,
    table_origin,
    table_size = arc_file.read(18).unpack('xxLx4L2')

    arc_file.seek(table_origin)
    num_files = table_size / 20

    meta = {}
    num_files.times do
      name_origin,
      name_size,
      data_origin,
      data_size = arc_file.read(20).unpack('L4 x4')

      name = arc_file.peek(name_origin) { arc_file.read(name_size - 1) }
      data = arc_file.peek(data_origin) { arc_file.read(data_size) }
      data, meta[name.gsub('\\', '/').to_sym] = decode(data)
      output_files.write(name, data)
    end

    meta.reject! { |_k, v| v.nil? }
    output_files.write_meta(meta) unless meta.empty?
  end

  def pack_internal(arc_file, input_files, _options)
    meta = input_files.read_meta || {}

    table_origin = MAGIC.length + 18
    table_size = input_files.length * 20

    version = 24
    arc_file.write(MAGIC)
    arc_file.write([version, table_origin, table_size].pack('xxLx4L2'))
    arc_file.write("\x00" * table_size)

    table_entries = {}
    input_files.each do |name, data|
      data = encode(name, data, meta[name.gsub('\\', '/').to_sym])

      table_entries[name] = { name_origin: arc_file.tell }
      arc_file.write(name.gsub('/', '\\'))
      arc_file.write("\x00")

      table_entries[name][:data_origin] = arc_file.tell
      table_entries[name][:data_size] = data.length
      arc_file.write(data)
    end

    arc_file.seek(table_origin)
    table_entries.each do |name, entry|
      arc_file.write([
        entry[:name_origin],
        name.length + 1,
        entry[:data_origin],
        entry[:data_size]].pack('L4 x4'))
    end
  end

  private

  def decode(data)
    if data[0..(YkgConverter::MAGIC.length - 1)] == YkgConverter::MAGIC
      data, regions = YkgConverter.decode(data)
      return [data, regions]
    end

    [data, nil]
  end

  def encode(file_name, data, file_meta)
    if file_name.downcase.end_with?('.ykg')
      regions = file_meta
      return YkgConverter.encode(data, regions)
    end

    data
  end
end

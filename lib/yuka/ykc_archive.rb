require_relative 'ykg_converter'
require_relative 'yks_converter'

# YKC archive
module YkcArchive
  MAGIC = 'YKC001'

  class Unpacker
    def unpack(arc_file, output_files, _options)
      magic = arc_file.read(MAGIC.length)
      fail 'Not a YKC archive' unless magic == MAGIC

      _version,
      table_origin,
      table_size = arc_file.read(18).unpack('xxLx4L2')

      arc_file.seek(table_origin)
      num_files = table_size / 20

      meta = {}
      num_files.times do
        output_files.write do
          name_origin,
          name_size,
          data_origin,
          data_size = arc_file.read(20).unpack('L4 x4')

          name = arc_file.peek(name_origin) { arc_file.read(name_size - 1) }
          data = arc_file.peek(data_origin) { arc_file.read(data_size) }
          data, meta[name.gsub('\\', '/').to_sym] = decode(data)

          [name, data]
        end
      end

      meta.reject! { |_k, v| v.nil? }
      output_files.write_meta(meta) unless meta.empty?
    end

    private

    def decode(data)
      meta = nil

      if data.start_with?(YkgConverter::MAGIC)
        data, meta = YkgConverter.decode(data)
      elsif data.start_with?(YksConverter::MAGIC)
        data = YksConverter.decode(data)
      end

      [data, meta]
    end
  end

  class Packer
    def pack(arc_file, input_files, _options)
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

    def encode(name, data, meta)
      if name.downcase.end_with?('.ykg')
        regions = meta
        data = YkgConverter.encode(data, regions)
      elsif name.downcase.end_with?('.yks')
        data = YksConverter.encode(data, true)
      end

      data
    end
  end
end

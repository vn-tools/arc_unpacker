require 'lib/formats/generic/pickle'
require 'zlib'

# RPA archive
# Engine: Ren'Py
# Extension: .rpa
# Known games:
# - Everlasting Summer
# - Katawa Shoujo
module RpaArchive
  module_function

  MAGIC3 = 'RPA-3.0 '
  MAGIC2 = 'RPA-2.0 '

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  # RPA archive packer
  class Packer
    def pack(arc_file, input_files, options)
      @arc_file = arc_file
      key = options[:key] || 0

      write_magic(key)
      header_pos = @arc_file.tell
      write_dummy_header(key)

      table = write_contents(input_files)
      table_pos = @arc_file.tell
      write_table(table, key)

      @arc_file.seek(header_pos)
      write_header(table_pos, key)
    end

    private

    def write_magic(key)
      @arc_file.write(key != 0 ? MAGIC3 : MAGIC2)
    end

    def write_dummy_header(key)
      write_header(0, key)
    end

    def write_header(table_pos, key)
      if key == 0
        @arc_file.write(format("%016x\n", table_pos))
      else
        @arc_file.write(format("%016x %08x\n", table_pos, key))
      end
    end

    def write_contents(input_files)
      table = []
      input_files.each do |name, data|
        table.push(origin: @arc_file.tell, size: data.length, name: name)
        @arc_file.write(data)
      end
      table
    end

    def write_table(table, key)
      raw = {}
      table.each do |e|
        tuple = [e[:origin] ^ key, e[:size] ^ key].freeze
        raw[e[:name]] = [tuple].freeze
      end
      @arc_file.write(Zlib.deflate(Pickle.dumps(raw)))
    end
  end

  # RPA archive unpacker
  class Unpacker
    def unpack(arc_file, output_files, _options)
      @arc_file = arc_file

      magic = assert_magic
      table_pos, key = read_header(magic)

      @arc_file.seek(table_pos)
      table = read_table
      table = decrypt_table(table, key)

      table.each do |name, options|
        output_files.write do
          data = options[:prefix]
          data += @arc_file.peek(options[:origin]) do
            @arc_file.read(options[:size])
          end
          [name, data]
        end
      end
    end

    private

    def assert_magic
      magic = @arc_file.read(MAGIC3.length)
      fail ArcError, 'Not a RPA archive' unless [MAGIC2, MAGIC3].include?(magic)
      magic
    end

    def read_header(magic)
      if magic == MAGIC3
        table_origin = @arc_file.read(16).to_i(16)
        @arc_file.skip(1)
        key = @arc_file.read(8).to_i(16)
      else
        table_origin = @arc_file.read(16).to_i(16)
        key = 0
      end
      [table_origin, key]
    end

    def read_table
      table = {}
      Pickle.loads(Zlib.inflate(@arc_file.read)).each do |k, v|
        table[k] = {
          origin: v[0][0],
          size: v[0][1],
          prefix: v[0][2] || ''
        }
      end
      table
    end

    def decrypt_table(table, key)
      table.each do |_, v|
        v[:origin] ^= key
        v[:size] ^= key
      end
      table
    end
  end
end

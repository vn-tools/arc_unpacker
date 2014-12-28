require_relative '../archive'
require_relative 'pickle'
require 'zlib'

# RPA archive
class RpaArchive < Archive
  MAGIC3 = 'RPA-3.0 '
  MAGIC2 = 'RPA-2.0 '

  def unpack_internal(arc_file, output_files, _options)
    RpaUnpacker.new.unpack(arc_file, output_files)
  end

  # RPA archive unpacker
  class RpaUnpacker
    def unpack(arc_file, output_files)
      @arc_file = arc_file

      fail_on_version2
      assert_version3
      table = read_table

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

    def fail_on_version2
      @arc_file.peek(0) do
        if @arc_file.read(MAGIC2.length) == MAGIC2
          fail \
            'This version is not yet supported. ' \
            'Please send this game title to rr- on github'
        end
      end
    end

    def assert_version3
      magic = @arc_file.read(MAGIC3.length)
      fail 'Not a RPA archive' unless magic == MAGIC3
    end

    def read_table
      table_origin = @arc_file.read(16).to_i(16)
      @arc_file.skip(1)
      key = @arc_file.read(8).to_i(16)

      @arc_file.peek(table_origin) do
        table = {}
        Pickle.loads(Zlib.inflate(@arc_file.read)).each do |k, v|
          table[k] = {
            origin: v[0][0],
            size: v[0][1],
            prefix: v[0][2]
          }
        end
        decrypt_table(table, key)
      end
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

require 'lib/binary_io'
require 'lib/image'

# Converts YKG to PNG and vice versa.
# Seen in YKC archives.
module YkgConverter
  module_function

  MAGIC = 'YKG000'

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    file.data = Decoder.new.read(file.data)
  end

  def encode!(file, _options)
    file.data = Encoder.new.write(file.data)
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)

      magic = input.read(MAGIC.length)
      fail RecognitionError, 'Not a YKG picture' if magic != MAGIC

      is_encrypted,
      _header_size,
      data_origin,
      data_size,
      meta_origin,
      meta_size = input.read(58).unpack('SL x28 LL x8 LL')
      is_encrypted = is_encrypted != 0

      fail 'Reading encrypted YKG is not supported.' if is_encrypted

      data = input.peek(data_origin) { input.read(data_size) }
      meta = input.peek(meta_origin) { input.read(meta_size) }

      data[1..3] = 'PNG' if data[1..3] == 'GNP'

      meta = BinaryIO.from_string(meta)
      regions = []
      (meta_size / 64).times do
        x, y, w, h = meta.read(64).unpack('L4 x48')
        regions.push(x: x, y: y, width: w, height: h)
      end

      raw_data, meta = Image.boxed_to_raw(data, 'RGBA')
      meta[:regions] = regions
      Image.raw_to_boxed(meta[:width], meta[:height], raw_data, 'RGBA', meta)
    end
  end

  class Encoder
    def write(data)
      regions = Image.read_meta_from_boxed(data)[:regions]
      output = BinaryIO.from_string
      meta = BinaryIO.from_string

      regions.each do |r|
        meta.write([r[:x], r[:y], r[:width], r[:height]].pack('L4 x48'))
      end
      meta.rewind
      meta = meta.read

      header_size = 0x40
      data_origin = header_size
      meta_origin = data_origin + data.length

      fail 'Only PNG files are supported' if data[1..3] != 'PNG'
      data[1..3] = 'GNP'

      output.write(MAGIC)
      output.write([
        header_size,
        data_origin,
        data.length,
        meta_origin,
        meta.length].pack('x2 L x28 LL x8 LL'))
      output.write(data)
      output.write(meta)

      output.rewind
      output.read
    end
  end
end

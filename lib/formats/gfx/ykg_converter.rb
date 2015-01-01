require 'lib/binary_io'

# Converts YKG to PNG and vice versa.
# Seen in YKC archives.
module YkgConverter
  module_function

  MAGIC = 'YKG000'

  def decode(data)
    input = BinaryIO.from_string(data)

    magic = input.read(MAGIC.length)
    fail 'Not a YKG picture' if magic != MAGIC

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

    [data, regions]
  end

  def encode(data, regions)
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

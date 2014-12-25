require_relative '../binary_io'

# Converts YKG to PNG and vice versa.
class YkgConverter
  MAGIC = 'YKG000'

  def self.decode(data)
    input = BinaryIO.from_string(data)

    magic = input.read(MAGIC.length)
    fail 'Not a YKG picture' if magic != MAGIC

    data_origin,
    data_size,
    meta_origin,
    meta_size = input.read(58).unpack('x2 x4 x28 LL x8 LL')

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

  def self.encode(data, regions)
    output = BinaryIO.from_string

    meta = BinaryIO.from_string
    regions.each do |r|
      meta.write([r[:x], r[:y], r[:width], r[:height]].pack('L4 x48'))
    end
    meta.rewind
    meta = meta.read

    data_origin = 0x40
    meta_origin = data_origin + data.length

    fail 'Only PNG files are supported' if data[1..3] != 'PNG'
    data[1..3] = 'GNP'

    output.write(MAGIC)
    output.write([
      0x40, # ?
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

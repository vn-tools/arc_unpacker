require 'lib/binary_io'

# Sound manipulation helper.
class Sound
  attr_accessor :channel_count
  attr_accessor :bytes_per_sample
  attr_accessor :sample_rate
  attr_accessor :samples

  def update_file(file)
    file.data = to_boxed
    file.change_extension('wav')
  end

  def to_boxed
    fail 'Empty sample data' if @samples.empty?

    _sample_count = @samples.length / @channel_count
    block_align = @channel_count * @bytes_per_sample
    byte_rate = block_align * @sample_rate
    bits_per_sample = @bytes_per_sample * 8

    output = BinaryIO.from_string('')
    output.write('RIFF')
    output.write("\x00" * 4)
    output.write('WAVE')
    output.write('fmt ')
    output.write([
      16,
      1,
      @channel_count,
      @sample_rate,
      byte_rate,
      block_align,
      bits_per_sample].pack('LSSLLSS'))

    output.write('data')
    output.write([@samples.length].pack('L'))
    output.write(@samples)

    output.seek(4)
    output.write([output.size - 8].pack('L'))
    output.rewind
    output.read
  end
end

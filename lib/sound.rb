require 'lib/binary_io'

# Sound manipulation helper.
module Sound
  module_function

  # Converts raw samples to chosen readable format, such as WAV.
  def raw_to_boxed(
    raw_sample_data,
    channel_count,
    bytes_per_sample,
    sample_rate)

    fail 'Empty sample data' if raw_sample_data.empty?

    _sample_count = raw_sample_data.length / channel_count
    block_align = channel_count * bytes_per_sample
    byte_rate = block_align * sample_rate
    bits_per_sample = bytes_per_sample * 8

    output = BinaryIO.from_string('')
    output.write('RIFF')
    output.write("\x00" * 4)
    output.write('WAVE')
    output.write('fmt ')
    output.write([
      16,
      1,
      channel_count,
      sample_rate,
      byte_rate,
      block_align,
      bits_per_sample].pack('LSSLLSS'))

    output.write('data')
    output.write([raw_sample_data.length].pack('L'))

    output.write(raw_sample_data)

    output.seek(4)
    output.write([output.size - 8].pack('L'))
    output.rewind
    output.read
  end
end

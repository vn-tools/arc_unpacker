require 'lib/binary_io'
require 'lib/sound'

# Converts NWA to WAV and vice versa.
# Seen in Key games:
# - Clannad
# - Little Busters
module NwaConverter
  module_function

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode(data, _options)
    Decoder.new.decode(data)
  end

  def encode(_data, _options)
    fail 'Not supported'
  end

  class Decoder
    def decode(data)
      input = BinaryIO.from_string(data)
      header = read_header(input)
      if header[:compression_level] == -1 \
        || header[:block_count] == 0 \
        || header[:compressed_size] == 0 \
        || header[:block_size] == 0 \
        || header[:rest_size] == 0
        samples = read_as_pcm(header, input)
      else
        validate_header(header)
        samples = read_samples(header, input)
      end
      Sound.raw_to_boxed(
        samples,
        header[:channel_count],
        header[:bits_per_sample] / 8,
        header[:sample_rate])
    end

    def read_header(input)
      header = {}
      header[:channel_count],
      header[:bits_per_sample],
      header[:sample_rate],
      header[:compression_level],
      header[:block_count],
      header[:uncompressed_size],
      header[:compressed_size],
      header[:sample_count],
      header[:block_size],
      header[:rest_size] = input.read(0x28).unpack('S2 L l L6')
      header
    end

    def read_as_pcm(header, input)
      input.read(header[:block_size] * header[:channel_count])
    end

    def read_samples(_header, _input)
      fail 'Not supported'
    end

    def validate_header(header)
      unless (0..5).include?(header[:compression_level])
        fail RecognitionError, 'Unsupported compression level.'
      end

      unless (1..2).include?(header[:channel_count])
        fail RecognitionError, 'Unsupported channel count.'
      end

      unless [8, 16].include?(header[:bits_per_sample])
        fail RecognitionError, 'Unsupported bits per sample.'
      end

      fail RecognitionError, 'No blocks found.' if header[:blocks] <= 0
      fail RecognitionError, 'No data found.' if header[:compressed_size] == 0

      if header[:uncompressed_size] !=
         header[:sample_count] * header[:bits_per_sample] / 8
        fail RecognitionError, 'Bad data size.'
      end

      if header[:sample_count] !=
         (header[:block_count] - 1) * header[:block_size] + header[:rest_size]
        fail RecognitionError, 'Bad sample count.'
      end

      true
    end
  end
end

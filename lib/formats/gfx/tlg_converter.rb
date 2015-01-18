require 'lib/binary_io'
require 'lib/image'
require_relative 'tlg_converter/tlg_pixel_decoder'

# Converts TLG to PNG.
# Seen in XP3 archives.
module TlgConverter
  module_function

  MAGIC_TLG0 = "TLG0.0\x00sds\x1a"
  MAGIC_TLG5 = "TLG5.0\x00raw\x1a"
  MAGIC_TLG6 = "TLG6.0\x00raw\x1a"

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  def decode!(file, _options)
    Decoder.new.read(file.data).update_file(file)
  end

  def encode!(_file, _options)
    fail 'Not supported'
  end

  class Decoder
    def read(data)
      input = BinaryIO.from_string(data)
      if data.start_with?(MAGIC_TLG0)
        input.seek(MAGIC_TLG0.length)
        return read_tlg0(input)
      elsif data.start_with?(MAGIC_TLG5)
        input.seek(MAGIC_TLG5.length)
        return read_tlg5(input)
      elsif data.start_with?(MAGIC_TLG6)
        input.seek(MAGIC_TLG6.length)
        return read_tlg6(input)
      else
        fail 'Not a TLG image.'
      end
    end

    def read_tlg0(input)
      raw_data_size = input.read(4).unpack('L')[0]
      image = read(input.read(raw_data_size))
      image.meta = { tags: {} }
      until input.eof?
        chunk_name = input.read(4)
        chunk_size = input.read(4).unpack('L')[0]
        chunk_data = input.read(chunk_size)
        if chunk_name == 'tags'
          extract_string = lambda do |container|
            str_length = container.to_i
            container = container[str_length.to_s.length + 1..-1]
            str = container[0..str_length - 1]
            container = container[str_length + 1..-1]
            next [str, container]
          end

          while chunk_data != ''
            key, chunk_data = extract_string.call(chunk_data)
            val, chunk_data = extract_string.call(chunk_data)
            image.meta[:tags][key.to_sym] = val
          end
        else
          STDERR.puts 'Unknown chunk: ' + chunk_name
        end
      end
      image
    end

    def read_tlg5(input)
      header = {}
      header[:channel_count],
      header[:image_width],
      header[:image_height],
      header[:block_height] = input.read(13).unpack('CL4')
      pixels = input.read

      output = decode_tlg5_pixels(header, pixels)
      Image.from_pixels(
        header[:image_width],
        header[:image_height],
        output,
        'RGBA')
    end

    def read_tlg6(input)
      header = {}
      header[:channel_count],
      header[:data_flags],
      header[:color_type],
      header[:external_golomb_table],
      header[:image_width],
      header[:image_height],
      header[:max_bit_length] = input.read(16).unpack('C4L3')
      pixels = input.read

      output = decode_tlg6_pixels(header, pixels)
      Image.from_pixels(
        header[:image_width],
        header[:image_height],
        output,
        'RGBA')
    end
  end
end

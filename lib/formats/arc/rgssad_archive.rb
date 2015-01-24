require 'lib/virtual_file'

# RGSSAD archive
# Engine: Ruby Game Scripting System
# Extension: .rgssad
# Known games:
# - Cherry Tree High Comedy Club
module RgssadArchive
  module_function

  MAGIC = "RGSSAD\x00"
  INITIAL_KEY = 0xdeadcafe

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  class Unpacker
    def unpack(arc_file, output_files, _options)
      magic = arc_file.read(MAGIC.length)
      fail RecognitionError, 'Not an XP3 archive' unless magic == MAGIC

      version = arc_file.read(1).unpack('C')[0]
      if version != 1
        fail 'Reading version ' + version.to_s + ' is not supported'
      end

      key = INITIAL_KEY
      table = read_table(arc_file, key)
      read_contents(arc_file, table, output_files)
    end

    def read_table(arc_file, key)
      table = []
      until arc_file.eof?
        name_length = arc_file.read(4).unpack('L<')[0]
        name_length ^= key
        key = RgssadArchive.advance_key(key)
        if name_length > arc_file.size
          fail RecognitionError, 'Bad file name length'
        end
        e = {}

        e[:name] = arc_file.read(name_length)
        name_length.times do |i|
          e[:name][i] = (e[:name][i].ord ^ key & 0xff).chr
          key = RgssadArchive.advance_key(key)
        end

        e[:size] = arc_file.read(4).unpack('L<')[0]
        e[:size] ^= key
        key = RgssadArchive.advance_key(key)

        e[:key] = key
        e[:origin] = arc_file.tell
        arc_file.skip(e[:size])

        if e[:origin] + e[:size] > arc_file.size
          fail RecognitionError, 'Bad offset to file'
        end

        table.push(e)
      end
      table
    end

    def read_contents(arc_file, table, output_files)
      table.each do |e|
        output_files.write do
          data = arc_file.peek(e[:origin]) { arc_file.read(e[:size]) }
          data = (data + "\x00\x00\x00\x00").unpack('L<*')
          key = e[:key]
          data.length.times do |i|
            data[i] ^= key
            key = RgssadArchive.advance_key(key)
          end
          data = data.pack('L<*')
          data = data[0, e[:size]]
          VirtualFile.new(e[:name], data)
        end
      end
    end
  end

  class Packer
    def pack(arc_file, input_files, _options)
      arc_file.write(MAGIC)
      arc_file.write("\x01")

      key = INITIAL_KEY
      input_files.each do |file|
        arc_file.write([file.name.length ^ key].pack('L<'))
        key = RgssadArchive.advance_key(key)

        name = file.name.gsub(/\//, '\\').unpack('C*')
        name.length.times do |i|
          name[i] ^= key & 0xff
          key = RgssadArchive.advance_key(key)
        end
        name = name.pack('C*')
        arc_file.write(name)

        arc_file.write([file.data.length ^ key].pack('L<'))
        key = RgssadArchive.advance_key(key)

        file_key = key
        data = (file.data + "\x00\x00\x00\x00").unpack('L<*')
        data.length.times do |i|
          data[i] ^= file_key
          file_key = RgssadArchive.advance_key(file_key)
        end
        data = data.pack('L<*')
        data = data[0, file.data.length]
        arc_file.write(data)
      end
    end
  end

  def advance_key(key)
    (key * 7 + 3) & 0xffff_ffff
  end
end

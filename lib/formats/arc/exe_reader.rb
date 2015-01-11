require 'lib/common'
silence_warnings { require 'pedump' }

# Windows executable reader
# Extension: .exe
# Known games:
# - Fortune Summoners
module ExeReader
  module_function

  def add_cli_help(_arg_parser) end

  def parse_cli_options(_arg_parser, _options) end

  class Unpacker
    def unpack(handle, output_files, _options)
      pedump = init_pedump(handle)
      warn_about_packer(pedump) unless pedump.packer.nil?

      fail RecognitionError, 'No resources found' if pedump.resources.nil?
      pedump
        .resources
        .reject { |resource| resource.file_offset.nil? }
        .each { |resource| write_resource(resource, handle, output_files) }
    end

    private

    def write_resource(pedump_resource, handle, output_files)
      output_files.write do
        file_name = pedump_resource.type + '/' + pedump_resource.name
        handle.seek(pedump_resource.file_offset)
        data = handle.read(pedump_resource.size)
        [file_name, data]
      end
    end

    def init_pedump(handle)
      pedump = PEdump.new(handle, log_level: PEdump::Logger::FATAL + 1)
      silence_warnings do
        pedump.dump
        pedump.resources
      end
      pedump
    end

    def warn_about_packer(pedump)
      pedump.logger.warn(format(
        '[!] Apparently, the .exe is packed with %s',
        pedump.packer.first.name))

      pedump.logger.warn(
        '[!] It is best to try unpacking the executable manually.')
    end
  end
end

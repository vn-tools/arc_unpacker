require_relative '../archive'
require_relative '../file_entry'
require_relative '../warning_silencer'
silence_warnings { require 'pedump' }

# Windows executable reader
class ExeReader < Archive
  def read_internal(handle)
    pedump = init_pedump(handle)
    warn_about_packer(pedump) unless pedump.packer.nil?
    @files = \
      pedump
      .resources
      .reject { |resource| resource.file_offset.nil? }
      .map { |resource| to_file_entry(resource) }
  end

  private

  def to_file_entry(pedump_resource)
    file_name = pedump_resource.type + '/' + pedump_resource.name
    data = lambda do
      input_file.seek(pedump_resource.file_offset, IO::SEEK_SET)
      input_file.read(pedump_resource.size)
    end
    FileEntry.new(file_name, data)
  end

  def init_pedump(handle)
    pedump = PEdump.new(handle)
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

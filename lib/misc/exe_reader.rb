require_relative '../archive'
require_relative '../warning_silencer'
silence_warnings { require 'pedump' }

# Windows executable reader
class ExeReader < Archive
  def unpack_internal(handle, output_files)
    pedump = init_pedump(handle)
    warn_about_packer(pedump) unless pedump.packer.nil?

    pedump
      .resources
      .reject { |resource| resource.file_offset.nil? }
      .each { |resource| write_resource(resource, handle, output_files) }
  end

  private

  def write_resource(pedump_resource, handle, output_files)
    file_name = pedump_resource.type + '/' + pedump_resource.name
    handle.seek(pedump_resource.file_offset, IO::SEEK_SET)
    data = handle.read(pedump_resource.size)
    output_files.write(file_name, data)
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

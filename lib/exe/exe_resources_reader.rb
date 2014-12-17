require 'pedump'
require_relative 'exe_resource_entry'

# Reader of .exe resources
class ExeResourcesReader
  attr_reader :files

  def initialize(path)
    pedump = init_pedump(path)
    warn_about_packer(pedump) unless pedump.packer.nil?
    @files = \
      pedump
      .resources
      .reject { |resource| resource.file_offset.nil? }
      .map { |resource| to_native_resource(resource) }
    self
  end

  private

  def to_native_resource(pedump_resource)
    ExeResourceEntry.new(
      pedump_resource.type + '/' + pedump_resource.name,
      pedump_resource.file_offset,
      pedump_resource.size)
  end

  def init_pedump(path)
    pedump = PEdump.new(path)
    pedump.dump
    pedump.resources
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

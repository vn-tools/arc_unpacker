#!/usr/bin/ruby -W2
require 'fileutils'
require_relative '../lib/cli'
require_relative '../lib/binary_io'
require_relative '../lib/input_files'

# CLI frontend
class ArchivePacker < CLI
  def run_internal
    fail 'Must specify output format.' if @options[:archive].nil?
    pack(@options[:archive], @options)
  end

  def pack(archive, options)
    fail 'Packing not supported' unless defined? archive::Packer
    if defined? archive.register_options
      archive.register_options(@arg_parser, options)
      @arg_parser.parse
    end

    input_files = InputFiles.new(options[:input_path], options)
    packer = archive::Packer.new
    BinaryIO.from_file(options[:output_path], 'wb') do |arc_file|
      packer.pack(arc_file, input_files, options)
    end
  end
end

ArchivePacker.new.run

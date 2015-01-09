#!/usr/bin/ruby -W2
require_relative '../lib/common'
require 'fileutils'
require 'lib/cli'
require 'lib/binary_io'
require 'lib/input_files'

# CLI frontend
class ArchivePacker < CLI
  def run_internal
    fail OptionError, 'Must specify output format.' if @options[:archive].nil?
    pack(@options[:archive], @options)
  end

  def pack(archive, options)
    fail 'Packing not supported' unless defined? archive::Packer
    archive.parse_cli_options(@arg_parser, options)

    input_files = InputFiles.new(options[:input_path], options)
    packer = archive::Packer.new
    BinaryIO.from_file(options[:output_path], 'wb') do |arc_file|
      packer.pack(arc_file, input_files, options)
    end
  end
end

ArchivePacker.new.run

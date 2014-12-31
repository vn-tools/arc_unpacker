#!/usr/bin/ruby -W2
require 'fileutils'
require_relative '../lib/cli'
require_relative '../lib/binary_io'
require_relative '../lib/input_files'

# CLI frontend
class ArchivePacker < CLI
  def run_internal
    BinaryIO.from_file(@options[:output_path], 'wb') do |arc_file|
      input_files = InputFiles.new(@options[:input_path], @options)
      fail 'Packing not supported' unless defined? @options[:archive]::Packer
      packer = @options[:archive]::Packer.new
      packer.pack(arc_file, input_files, @options)
    end
  end
end

ArchivePacker.new.run

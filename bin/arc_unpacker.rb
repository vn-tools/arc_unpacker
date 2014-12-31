#!/usr/bin/ruby -W2
require 'fileutils'
require_relative '../lib/cli'
require_relative '../lib/binary_io'
require_relative '../lib/output_files'

# CLI frontend
class ArchiveUnpacker < CLI
  def run_internal
    BinaryIO.from_file(@options[:input_path], 'rb') do |arc_file|
      FileUtils.mkpath(@options[:output_path])
      output_files = OutputFiles.new(@options[:output_path], @options)
      unless defined? @options[:archive]::Unpacker
        fail 'Unpacking not supported'
      end
      unpacker = @options[:archive]::Unpacker.new
      unpacker.unpack(arc_file, output_files, @options)
    end
  end
end

ArchiveUnpacker.new.run

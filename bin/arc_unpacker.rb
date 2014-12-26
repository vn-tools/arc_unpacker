#!/usr/bin/ruby -W2
require 'fileutils'
require_relative '../lib/cli'

# CLI frontend
class ArchiveUnpacker < CLI
  def run_internal
    FileUtils.mkpath(@options[:output_path])
    archive = ArchiveFactory.get(@options[:format])
    archive.unpack(
      @options[:input_path],
      @options[:output_path],
      @options)
  end
end

ArchiveUnpacker.new.run

#!/usr/bin/ruby -W2
require 'fileutils'
require_relative '../lib/cli'

# CLI frontend
class ArchivePacker < CLI
  def run_internal
    FileUtils.mkpath(File.dirname(@options[:output_path]))
    archive = ArchiveFactory.get(@options[:format])
    archive.pack(
      @options[:input_path],
      @options[:output_path],
      @options)
  end

  def usage_fmt
    'Usage: %s INPUT_DIR OUTPUT_PATH'
  end
end

ArchivePacker.new.run

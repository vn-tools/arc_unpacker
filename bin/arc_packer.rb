#!/usr/bin/ruby -W2
require 'fileutils'
require_relative '../lib/cli'

# CLI frontend
class ArchivePacker < CLI
  def run_internal
    FileUtils.mkpath(File.dirname(@options[:output_path]))
    archive = ArchiveFactory.get(@options[:format])
    archive.add_files(@options[:input_path])
    archive.write(@options[:output_path])
  end

  def usage_fmt
    'Usage: %s INPUT_DIR OUTPUT_PATH'
  end
end

ArchivePacker.new.run

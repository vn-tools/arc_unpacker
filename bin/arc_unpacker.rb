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

  def usage_fmt
    'Usage: %s INPUT_PATH OUTPUT_DIR'
  end
end

ArchiveUnpacker.new.run

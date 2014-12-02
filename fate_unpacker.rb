#!/usr/bin/ruby -W2
require_relative 'lib/fate_unpacker'
require 'ostruct'
require 'optparse'
require 'fileutils'

# CLI frontend for Fate unpacker
class CLI
  def initialize
    parse_options
  end

  def run
    FileUtils.mkpath(@options.output_path) \
      unless @options.output_path.nil?

    FateUnpacker.new(@options.input_path).extract(@options.output_path)
  end

  private

  def parse_options
    @options = OpenStruct.new
    @options.input_path = nil
    @options.output_path = nil

    opt_parser = OptionParser.new do |opts|
      opts.banner = format(
        'Usage: %s PATH_TO_XP3 OUTPUT_DIR',
        File.basename(__FILE__))

      opts.separator ''

      opts.on_tail('-h', '--help', 'Show this message') do
        puts opts
        exit
      end
    end

    begin
      opt_parser.parse!
    rescue OptionParser::InvalidOption => e
      puts e.message
      puts
      puts opt_parser
      exit(1)
    end

    if ARGV.length != 2
      puts opt_parser
      exit(1)
    end

    @options.input_path = ARGV.first
    @options.output_path = ARGV.last
  end

  def escape(string)
    string
      .sub(/^\//, '')
      .gsub(/([^ a-zA-Z0-9_.-]+)/n, '_')
  end
end

CLI.new.run

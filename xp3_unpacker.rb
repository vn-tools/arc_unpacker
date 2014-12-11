#!/usr/bin/ruby -W2
require_relative 'decryptors/noop'
require_relative 'decryptors/fsn'
require_relative 'decryptors/cxdec'
require_relative 'decryptors/cxdec_plugin_fha'
require_relative 'xp3_archive'
require 'ostruct'
require 'optparse'
require 'fileutils'

# CLI frontend for XP3 unpacker
class CLI
  def initialize
    parse_options
  end

  def run
    FileUtils.mkpath(@options.output_path) \
      unless @options.output_path.nil?

    decryptor = decryptors[@options.format].call

    xp3 = Xp3Archive.new(@options.input_path, decryptor)
    xp3.read!
    xp3.extract(@options.output_path, @options.verbose)
  end

  private

  def decryptors
    {
      noop: ->() { NoopDecryptor.new },
      fsn: ->() { FsnDecryptor.new },
      fha: ->() { CxdecDecryptor.new(CxdecPluginFha.new) }
    }
  end

  def parse_options
    @options = OpenStruct.new
    @options.input_path = nil
    @options.output_path = nil
    @options.verbose = true

    opt_parser = OptionParser.new do |opts|
      opts.banner = format(
        'Usage: %s PATH_TO_XP3 OUTPUT_DIR',
        File.basename(__FILE__))

      opts.separator ''

      opts.on('-q', '--quiet', 'Suppress output') do
        @options.verbose = false
      end

      opts.on(
        '-f',
        '--fmt FORMAT',
        decryptors.keys,
        'Select decryption type (' + decryptors.keys.join(', ') + ')') \
      do |format|
        @options.format = format
      end

      opts.on_tail('-h', '--help', 'Show this message') do
        puts opts
        exit
      end
    end

    begin
      opt_parser.parse!
      if @options.format.nil?
        fail OptionParser::MissingArgument, 'Must specify decryption format.'
      end
    rescue StandardError => e
      raise e unless e.class.name.start_with?('OptionParser')
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

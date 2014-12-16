#!/usr/bin/ruby -W2
require 'optparse'
require 'fileutils'

require_relative '../lib/kirikiri/decryptors/noop'
require_relative '../lib/kirikiri/decryptors/fsn'
require_relative '../lib/kirikiri/decryptors/cxdec'
require_relative '../lib/kirikiri/decryptors/cxdec_plugin_fha'
require_relative '../lib/kirikiri/xp3_archive'

# CLI frontend
class CLI
  def initialize
    parse_options
  end

  def run
    FileUtils.mkpath(@options[:output_path]) unless @options[:output_path].nil?
    begin
      run_internal
    rescue StandardError => e
      puts 'Error: ' + e.message.to_s
      exit(1)
    end
  end

  private

  def decryptors
    {
      'xp3/noop' => ->() { NoopDecryptor.new },
      'xp3/fsn' => ->() { FsnDecryptor.new },
      'xp3/fha' => ->() { CxdecDecryptor.new(CxdecPluginFha.new) }
    }
  end

  def run_internal
    decryptor = decryptors[@options[:format]].call

    archive = Xp3Archive.new(@options[:input_path], decryptor)
    archive.read!
    archive.extract(@options[:output_path], @options[:verbose])
  end

  def parse_options
    @options =
    {
      input_path: nil,
      output_path: nil,
      verbose: true
    }

    opt_parser = OptionParser.new do |opts|
      opts.banner = format(
        'Usage: %s INPUT_PATH OUTPUT_DIR',
        File.basename(__FILE__))

      opts.on(
        '-f',
        '--fmt FORMAT',
        decryptors.keys,
        'Select decryption type (' + decryptors.keys.join(', ') + ')') \
      do |format|
        @options[:format] = format
      end

      opts.separator ''
      opts.on('-q', '--quiet', 'Suppress output') { @options[:verbose] = false }
      opts.on_tail('-h', '--help', 'Show this message') { puts opts && exit }
    end

    begin
      opt_parser.parse!

      if @options[:format].nil?
        fail OptionParser::MissingArgument, 'Must specify decryption format.'
      end
    rescue StandardError => e
      raise e unless e.class.name.start_with?('OptionParser')
      puts e.message, '', opt_parser
      exit(1)
    end

    if ARGV.length != 2
      puts opt_parser
      exit(1)
    end

    @options[:input_path] = ARGV.first
    @options[:output_path] = ARGV.last
  end
end

CLI.new.run

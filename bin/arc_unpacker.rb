#!/usr/bin/ruby -W2
require 'optparse'
require 'fileutils'

require_relative '../lib/kirikiri/decryptors/noop'
require_relative '../lib/kirikiri/decryptors/fsn'
require_relative '../lib/kirikiri/decryptors/cxdec'
require_relative '../lib/kirikiri/decryptors/cxdec_plugin_fha'
require_relative '../lib/kirikiri/xp3_archive'
require_relative '../lib/ykc/ykc_archive'
require_relative '../lib/nscripter/nsa/nsa_archive'
require_relative '../lib/nscripter/sar/sar_archive'
require_relative '../lib/french_bread/melty_blood/melty_blood_archive'

def archive_factory
  {
    'xp3/noop' => -> { Xp3Archive.new(NoopDecryptor.new) },
    'xp3/fsn' => -> { Xp3Archive.new(FsnDecryptor.new) },
    'xp3/fha' => -> { Xp3Archive.new(CxdecDecryptor.new(CxdecPluginFha.new)) },
    'ykc' => -> { YkcArchive.new },
    'sar' => -> { SarArchive.new },
    'nsa' => -> { NsaArchive.new },
    'melty_blood' => -> { MeltyBloodArchive.new }
  }
end

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

  def run_internal
    archive = archive_factory[@options[:format]].call
    archive.read(@options[:input_path])
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
        archive_factory.keys,
        'Select decryption type (' + archive_factory.keys.join(', ') + ')') \
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

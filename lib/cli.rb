require 'optparse'
require_relative 'archive_factory'

# Generic CLI frontend
class CLI
  def initialize
    @options =
    {
      input_path: nil,
      output_path: nil,
      verbosity: :normal
    }

    parse_options
  end

  def run
    run_internal
  rescue StandardError => e
    puts 'Error: ' + e.message.to_s if @options[:verbosity] != :quiet
    puts e.backtrace if @options[:verbosity] == :debug
    exit(1)
  end

  def parse_options
    opt_parser = OptionParser.new do |opts|
      opts.banner = format(
        usage_fmt,
        File.basename(__FILE__))

      opts.on(
        '-f',
        '--fmt FORMAT',
        ArchiveFactory.format_strings,
        format(
          'Select encryption type (%s)',
          ArchiveFactory.format_strings.join(', '))) \
      do |format|
        @options[:format] = format
      end

      opts.separator ''

      opts.on('-q', '--quiet', 'Suppress output') do
        @options[:verbosity] = :quiet
      end

      opts.on('-v', '--verbose', 'Show debug information for errors') do
        @options[:verbosity] = :debug
      end

      opts.on_tail('-h', '--help', 'Show this message') do
        puts opts
        exit
      end
    end

    begin
      opt_parser.parse!

      if @options[:format].nil?
        fail OptionParser::MissingArgument, 'Must specify encryption format.'
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

require_relative 'archive_factory'
require_relative 'arg_parser'

# Generic CLI frontend
class CLI
  def initialize
    @options =
    {
      input_path: nil,
      output_path: nil,
      verbosity: :normal
    }
  end

  def run
    @arg_parser = ArgParser.new(ARGV)
    parse_options
    run_internal
  rescue OptionError => e
    puts 'Error: ' + e.message.to_s
    puts e.backtrace if @options[:verbosity] == :debug
    puts
    print_help
    exit(1)
  rescue StandardError => e
    puts 'Error: ' + e.message if @options[:verbosity] != :quiet
    puts e.backtrace if @options[:verbosity] == :debug
    exit(1)
  end

  def parse_options
    @arg_parser.on('-q', '--quiet') { @options[:verbosity] = :quiet }
    @arg_parser.on('-v', '--verbose') { @options[:verbosity] = :debug }

    @arg_parser.on('-h', '--help') do
      print_help
      exit(0)
    end

    @arg_parser.get('-f', '--fmt') do |format|
      unless ArchiveFactory.format_strings.include?(format)
        fail 'Unknown archive format'
      end

      archive = ArchiveFactory.get(format)
      archive.request_options(@arg_parser, @options)
      @arg_parser.parse
      @options[:format] = format
      @options[:archive] = archive
    end

    @arg_parser.get_stray { |value| @options[:input_path] = value }
    @arg_parser.get_stray { |value| @options[:output_path] = value }

    @arg_parser.parse
  end

  def print_help
    puts format(
      'Usage: %s [options] [arc_options] input_path output_path',
      File.basename(__FILE__))

    puts
    puts '[options] can be:'
    puts
    puts '-f, --fmt [format]   Select archive format.'
    puts '-q, --quiet          Suppress output.'
    puts '-v, --verbose        Show exception stack traces for debug purposes.'
    puts '-h, --help           Show this message.'
    puts
    puts '[arc_options] depend on each archive and are required at runtime.'
    puts
    puts '[format] currently supports:'
    puts
    puts ArchiveFactory.format_strings * "\n"
  end
end

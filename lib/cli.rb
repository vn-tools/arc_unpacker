require 'lib/archive_factory'
require 'lib/arg_parser'

# Generic CLI frontend
class CLI
  def initialize
    @options =
    {
      input_path: nil,
      output_path: nil,
      verbosity: :normal
    }
    @arg_parser = ArgParser.new(ARGV)
  end

  def run
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
    @arg_parser.add_help('-f, --fmt=FORMAT', 'Selects the archive format.')
    @arg_parser.add_help('-q, --quiet', 'Suppresses output.')
    @arg_parser.add_help('-v, --verbose', 'Shows additional debug information.')
    @arg_parser.add_help('-h, --help', 'Shows this message.')
    @options[:verbosity] = :quiet if @arg_parser.flag?(%w(-q --quiet))
    @options[:verbosity] = :debug if @arg_parser.flag?(%w(-v --verbose))
    @options[:format] = @arg_parser.switch(%w(-f --fmt))

    unless @options[:format].nil?
      unless  ArchiveFactory.format_strings.include?(@options[:format])
        fail 'Unknown archive format'
      end
      @options[:archive] = ArchiveFactory.get(@options[:format])
    end

    if @arg_parser.flag?(%w(-h  --help))
      print_help
      exit(0)
    end

    stray = @arg_parser.stray
    fail OptionError, 'Required more arguments.' if stray.count < 1
    @options[:input_path],
    @options[:output_path] = stray
    @options[:output_path] ||= @options[:input_path] + '~'
  end

  def print_help
    puts format(
      'Usage: %s [options] [arc_options] input_path [output_path]',
      File.basename($PROGRAM_NAME))

    puts \
      '',
      'Unless output path is provided, the script is going to use input_path',
      'followed with a tilde (~).',
      '',
      '[options] can be:',
      ''

    @arg_parser.print_help
    @arg_parser.clear_help
    puts

    if @options[:archive].nil?
      puts '[arc_options] depend on each archive and are required at runtime.'
      puts 'See --help --fmt FORMAT to see detailed help for given archive.'
    elsif defined? @options[:archive].add_cli_help
      @options[:archive].add_cli_help(@arg_parser)

      puts '[arc_options] specific to ' + @options[:format] + ':'
      puts
      @arg_parser.print_help
    end
  end
end

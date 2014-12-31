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
    register_basic_options
    @arg_parser.get_stray { |value| @options[:input_path] = value }
    @arg_parser.get_stray { |value| @options[:output_path] = value }
    @arg_parser.parse
  end

  def register_basic_options
    @arg_parser.on(
      '-q',
      '--quiet',
      'Suppress output.') do
      @options[:verbosity] = :quiet
    end

    @arg_parser.on(
      '-v',
      '--verbose',
      'Show exception stack traces for debug purposes.') do
      @options[:verbosity] = :debug
    end

    @arg_parser.on('-f', '--fmt', 'Select archive format.') do |format|
      unless ArchiveFactory.format_strings.include?(format)
        fail 'Unknown archive format'
      end

      archive = ArchiveFactory.get(format)
      @options[:format] = format
      @options[:archive] = archive
    end

    @arg_parser.on('-h', '--help', 'Show this message.') do
      print_help
      exit(0)
    end
  end

  def print_help
    puts format(
      'Usage: %s [options] [arc_options] input_path output_path',
      File.basename($PROGRAM_NAME))

    puts
    puts '[options] can be:'
    puts

    @arg_parser.print_help
    puts

    if @options[:archive].nil?
      puts '[arc_options] depend on each archive and are required at runtime.'
      puts 'See --help --fmt FORMAT to see detailed help for given archive.'
    elsif defined? @options[:archive].register_options
      @options[:archive].register_options(@arg_parser, nil)

      puts '[arc_options] specific to ' + @options[:format] + ':'
      @arg_parser.print_help
    end
  end
end

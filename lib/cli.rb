require 'lib/arg_parser'

# Generic CLI frontend
class CLI
  def initialize
    @options = { verbosity: :normal }
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
    @arg_parser.add_help('-q, --quiet', 'Suppresses output.')
    @arg_parser.add_help('-v, --verbose', 'Shows additional debug information.')
    @arg_parser.add_help('-h, --help', 'Shows this message.')
    @options[:verbosity] = :quiet if @arg_parser.flag?(%w(-q --quiet))
    @options[:verbosity] = :debug if @arg_parser.flag?(%w(-v --verbose))

    return unless @arg_parser.flag?(%w(-h  --help))
    print_help
    exit(0)
  end

  def print_help
    puts \
      'Unless output path is provided, the script is going to use input_path',
      'followed with a tilde (~).',
      '',
      '[options] can be:',
      ''

    @arg_parser.print_help
    @arg_parser.clear_help
    puts
  end
end

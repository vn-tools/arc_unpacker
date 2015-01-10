require 'lib/cli'
require 'lib/factory/archive_factory'

# CLI frontend for archive packed and unpacker
class CLIArchive < CLI
  def initialize
    super
    @options[:input_path] = nil
    @options[:output_path] = nil
  end

  def parse_options
    @arg_parser.add_help('-f, --fmt=FORMAT', 'Selects the archive format.')
    @options[:format] = @arg_parser.switch(%w(-f --fmt))

    unless @options[:format].nil?
      unless  ArchiveFactory.format_strings.include?(@options[:format])
        fail 'Unknown archive format'
      end
      @options[:archive] = ArchiveFactory.get(@options[:format])
    end

    super

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
    puts ''

    super

    if @options[:archive].nil?
      puts '[arc_options] depend on each archive and are required at runtime.'
      puts 'See --help --fmt FORMAT to see detailed help for given archive.'
    else
      @options[:archive].add_cli_help(@arg_parser)
      puts '[arc_options] specific to ' + @options[:format] + ':'
      puts
      @arg_parser.print_help
    end
  end
end

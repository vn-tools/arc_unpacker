#!/usr/bin/ruby -W2
require_relative '../lib/common'
require 'fileutils'
require 'lib/binary_io'
require 'lib/cli'
require 'lib/factory/converter_factory'
require 'lib/virtual_file'

# CLI frontend
class FileDecoder < CLI
  def run_internal
    verbose = @options[:verbosity] != :quiet
    expand_paths(@options[:input_paths]).each do |input_path|
      begin
        print 'Converting ' + input_path  + '... ' if verbose
        choose_converter_and_decode(input_path, @options)
        puts 'ok' if verbose
      rescue => e
        puts e.message if verbose
        if @options[:verbosity] == :debug
          puts
          puts e.backtrace
          puts
        end
        next
      end
    end
  end

  def choose_converter_and_decode(input_path, options)
    verbose = options[:verbosity] != :quiet

    unless options[:converter].nil?
      decode(input_path, options[:converter], options)
      return
    end

    ConverterFactory.each do |key, converter|
      print format('Trying %s... ', key) if verbose
      begin
        decode(input_path, converter, options)
        return
      rescue OptionError, RecognitionError => e
        puts e.message if verbose
        next
      end
    end
  end

  def decode(input_path, converter, options)
    converter.parse_cli_options(@arg_parser, options)

    if @options[:output_path].nil?
      output_path = input_path + '~'
    else
      output_path = File.join(options[:output_path], File.basename(input_path))
    end

    if File.exist?(output_path) && !@options[:overwrite]
      fail 'File already exists'
    end

    dir_path = File.dirname(output_path)
    file_name = File.basename(output_path)
    file = VirtualFile.new(file_name, File.binread(input_path))
    converter.decode!(file, options)

    FileUtils.mkpath(dir_path)
    File.binwrite(File.join(dir_path, file.name), file.data)
  end

  def parse_options
    @arg_parser.add_help('-f, --fmt=FORMAT', 'Selects the file format.')
    @arg_parser.add_help('-o, --out=FOLDER', 'Where to put the output files.')
    @arg_parser.add_help('--overwrite', 'Overwrites existing target files.')
    @options[:format] = @arg_parser.switch(%w(-f --fmt))
    @options[:output_path] = @arg_parser.switch(%w(-o --out))
    @options[:overwrite] = @arg_parser.flag?(%w(--overwrite))

    unless @options[:format].nil?
      unless ConverterFactory.format_strings.include?(@options[:format])
        fail 'Unknown file format'
      end
      @options[:converter] = ConverterFactory.get(@options[:format])
    end

    super

    fail OptionError, 'Required more arguments' if @arg_parser.stray.count < 1
    @options[:input_paths] = @arg_parser.stray
  end

  def print_help
    puts \
      format(
        'Usage: %s [options] [file_options] input_path [input_path...]',
        File.basename($PROGRAM_NAME)),
      '',
      '[options] can be:',
      ''

    @arg_parser.print_help
    @arg_parser.clear_help
    puts

    if @options[:converter].nil?
      puts '[file_options] depend on chosen format and are required at runtime.'
      puts 'See --help --fmt FORMAT to see detailed help for given converter.'
    else
      @options[:converter].add_cli_help(@arg_parser)
      puts '[file_options] specific to ' + @options[:format] + ':'
      puts
      @arg_parser.print_help
    end
  end
end

FileDecoder.new.run

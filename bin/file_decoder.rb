#!/usr/bin/ruby -W2
require_relative '../lib/common'
require 'fileutils'
require 'lib/cli'
require 'lib/binary_io'
require 'lib/factory/converter_factory'

# CLI frontend
class FileDecoder < CLI
  def run_internal
    verbose = @options[:verbosity] != :quiet

    unless @options[:converter].nil?
      decode(@options[:converter], @options)
      return
    end

    ConverterFactory.each do |key, converter|
      @options[:format] = key
      @options[:converter] = converter
      print format('Trying %s... ', key) if verbose
      begin
        decode(converter, @options)
        puts 'ok' if verbose
        return
      rescue => e
        puts e.message if verbose
        if @options[:verbosity] == :debug && !e.is_a?(RecognitionError)
          puts
          puts e.backtrace
          puts
        end
        next
      end
    end
  end

  def decode(converter, options)
    converter.parse_cli_options(@arg_parser, options)

    FileUtils.mkpath(File.dirname(options[:output_path]))
    data = File.binread(options[:input_path])
    data = converter.decode(data, options)
    File.binwrite(options[:output_path], data)
  end

  def parse_options
    @arg_parser.add_help('-f, --fmt=FORMAT', 'Selects the file format.')
    @options[:format] = @arg_parser.switch(%w(-f --fmt))

    unless @options[:format].nil?
      unless ConverterFactory.format_strings.include?(@options[:format])
        fail 'Unknown file format'
      end
      @options[:converter] = ConverterFactory.get(@options[:format])
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
      'Usage: %s [options] [file_options] input_path [output_path]',
      File.basename($PROGRAM_NAME))
    puts ''

    super

    if @options[:archive].nil?
      puts '[file_options] depend on each archive and are required at runtime.'
      puts 'See --help --fmt FORMAT to see detailed help for given archive.'
    else
      @options[:archive].add_cli_help(@arg_parser)
      puts '[file_options] specific to ' + @options[:format] + ':'
      puts
      @arg_parser.print_help
    end
  end
end

FileDecoder.new.run

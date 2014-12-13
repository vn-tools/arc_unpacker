#!/usr/bin/ruby -W2
require 'bundler/setup'
require 'vntools_helpers'
require_relative 'decryptors/noop'
require_relative 'decryptors/fsn'
require_relative 'decryptors/cxdec'
require_relative 'decryptors/cxdec_plugin_fha'
require_relative 'xp3_archive'

# CLI frontend for XP3 unpacker
class CLI < GenericUnpacker
  def run_internal
    decryptor = decryptors[@options[:format]].call

    archive = Xp3Archive.new(@options[:input_path], decryptor)
    archive.read!
    archive.extract(@options[:output_path], @options[:verbose])
  end

  private

  def decryptors
    {
      noop: ->() { NoopDecryptor.new },
      fsn: ->() { FsnDecryptor.new },
      fha: ->() { CxdecDecryptor.new(CxdecPluginFha.new) }
    }
  end

  def after_options_created(opts)
    opts.on(
      '-f',
      '--fmt FORMAT',
      decryptors.keys,
      'Select decryption type (' + decryptors.keys.join(', ') + ')') \
    do |format|
      @options[:format] = format
    end
  end

  def after_options_parsed
    if @options[:format].nil?
      fail OptionParser::MissingArgument, 'Must specify decryption format.'
    end
  end
end

CLI.new.run

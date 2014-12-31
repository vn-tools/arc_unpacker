# Generic error used by argument parser.
class OptionError < StandardError
  def initialize(message)
    super
  end
end

# Better argument parser.
# The key reason for using this over built-in OptParser is that it can be
# decorated dynamically without weird hacks.
class ArgParser
  attr_reader :stray

  def initialize(args)
    @switches = {}
    @flags = []
    @stray = []
    @help_entries = []
    parse(args)
  end

  def add_help(invocation, description, possible_values: [])
    unless possible_values.empty?
      description += " Possible values:\n"
      description += possible_values.map { |e| format('- %s', e) } * "\n"
    end
    @help_entries.push(invocation: invocation, description: description)
  end

  def clear_help
    @help_entries = []
  end

  def switch(keys)
    [*keys].each do |key|
      value = @switches[strip_dashes(key)]
      next if value.nil?
      return value
    end
    nil
  end

  def flag?(keys)
    keys = [*keys].map { |k| strip_dashes(k) }
    @flags.any? { |f| keys.include?(f) }
  end

  def print_help
    if @help_entries.empty?
      puts 'No additional switches are available.'
      return
    end

    @help_entries.each do |entry|
      left = entry[:invocation]
      right = entry[:description]

      left_size = 25
      right_size = 78 - left_size

      print left.ljust(left_size)
      puts word_wrap(right, right_size) * ("\n" + ' ' * left_size)
    end
  end

  private

  def parse(args)
    args.each do |arg|
      match = /\A--?(?<key>\w+)=(?<value>.*)\Z/.match(arg)
      if match
        @switches[match[:key]] = match[:value]
        next
      end

      match = /\A--?(?<key>\w+)\Z/.match(arg)
      if match
        @flags.push(match[:key])
        next
      end

      @stray.push(arg)
    end
  end

  def word_wrap(message, width)
    message.scan(/\S.{0,#{width}}\S(?=\s|$)|\S+/)
  end

  def myfail(message)
    fail OptionError, message
  end

  def strip_dashes(key)
    (key || '').gsub(/\A-+/, '')
  end
end

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
  def initialize(args)
    @getters = []
    @help_entries = []
    @args = args
  end

  def on(short, long, help_message = nil, values = nil, &block)
    @help_entries.push(
      short: short,
      long: long,
      message: help_message,
      parameters: block.parameters,
      mandatory: false)

    get_internal(short, long, false, values, &block)
  end

  def get(short, long, help_message = nil, values = nil, &block)
    @help_entries.push(
      short: short,
      long: long,
      message: help_message,
      parameters: block.parameters,
      mandatory: true)

    get_internal(short, long, true, values, &block)
  end

  def on_stray(&block)
    get_stray_internal(false, &block)
  end

  def get_stray(&block)
    get_stray_internal(true, &block)
  end

  def parse
    getters = @getters.dup
    @getters = []
    getters.each(&:call)
  end

  def print_help
    if @help_entries.empty?
      puts 'No additional switches are available.'
      return
    end

    @help_entries.each do |entry|
      params = entry[:parameters].map { |p| p[1] }
      params = params.map { |p| format('[%s]', p) } unless entry[:mandatory]
      switches =
        ['-' + strip_dashes(entry[:short]), '--' + strip_dashes(entry[:long])]
        .select { |m| strip_dashes(m).length > 0 }

      left = format('%s %s', switches.join(', '), params.join(', ').upcase)
      right = entry[:message]

      left_size = 25
      right_size = 78 - left_size

      print left.ljust(left_size)
      puts word_wrap(right, right_size) * ("\n" + ' ' * left_size)
    end
  end

  private

  def word_wrap(message, width)
    message.scan(/\S.{0,#{width}}\S(?=\s|$)|\S+/)
  end

  def myfail(message)
    fail OptionError, message
  end

  def get_internal(short, long, mandatory, allowed_values, &block)
    @getters.push(lambda do
      index = \
        @args.index('-' + strip_dashes(short)) || \
        @args.index('--' + strip_dashes(long))

      if index.nil?
        myfail(format('Required argument %s is missing.', long)) if mandatory
        return
      end

      indices = @args.each_index.select do |i|
        i > index && !@args[i].start_with?('-')
      end.first(block.arity)

      if mandatory && indices.length < block.arity
        myfail(format('Required values for %s are missing.', long))
      end

      values = @args.values_at(*indices)
      @args.delete_if.with_index { |_, i| indices.include?(i) }
      @args.delete_at(index)

      unless allowed_values.nil?
        fail 'This doesn\'t make sense!' if block.arity != 1
        unless allowed_values.include?(values.first) \
        || allowed_values.include?(values.first.to_sym)
          myfail(format(
            "Bad value %s for %s. Allowed values:\n%s",
            values.first,
            long,
            allowed_values * "\n"))
        end
      end

      block.call(*values)
    end)
  end

  def get_stray_internal(mandatory, &block)
    @getters.push(lambda do
      fail 'This doesn\'t make sense!' if block.arity == 0

      indices = @args.each_index.select do |i|
        !@args[i].start_with?('-')
      end.first(block.arity)

      if mandatory && indices.length < block.arity
        myfail('Required arguments are missing.')
      end

      values = @args.values_at(*indices)
      @args.delete_if.with_index { |_, i| indices.include?(i) }

      block.call(*values)
    end)
  end

  def strip_dashes(key)
    (key || '').gsub(/\A-+/, '')
  end
end

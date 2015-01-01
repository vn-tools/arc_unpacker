#!/usr/bin/ruby
$LOAD_PATH.unshift(File.join(File.dirname(__FILE__)))

Dir['test/**/*.rb'].each { |file| require_relative file }

#!/usr/bin/ruby
Dir['test/**/*.rb'].each { |file| require_relative file }

# frozen_string_literal: true

require "test_helper"

class HelloTest < Test::Unit::TestCase
  test "VERSION" do
    assert do
      ::Hello.const_defined?(:VERSION)
    end
  end

  test "say Hello" do
    hello = Hello::Hello.new
    actual = hello.say
    expected = "Hello Ruby"
    assert_equal(expected, actual)
  end
end

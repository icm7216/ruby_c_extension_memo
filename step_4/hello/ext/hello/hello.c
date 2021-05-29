#include "hello.h"

static VALUE
hello_say(VALUE self)
{
  return rb_str_new_cstr("Hello Ruby");
}

void
Init_hello(void)
{
  VALUE mHello;  
  VALUE cHello;

  mHello = rb_define_module("Hello");
  cHello = rb_define_class_under(mHello, "Hello", rb_cObject);
  rb_define_method(cHello, "say", hello_say, 0);
}

#include <stdlib.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"

#include "redismodule.h"

typedef struct redis_mruby_t {
  mrb_state *mrb;
} redis_mruby;

mrb_value redis_mruby_command_call(mrb_state *mrb, mrb_value self);
mrb_value redis_mruby_command_pcall(mrb_state *mrb, mrb_value self);

void redis_mruby_init(redis_mruby *rm) {
  struct RClass *class;
  class = mrb_define_class(rm->mrb, "Redis", rm->mrb->object_class);

  mrb_define_class_method(rm->mrb, class, "call", redis_mruby_command_call, MRB_ARGS_REQ(1));
  mrb_define_class_method(rm->mrb, class, "pcall", redis_mruby_command_pcall, MRB_ARGS_REQ(1));
}

redis_mruby *new_redis_mruby()
{
  redis_mruby *rm;

  rm = malloc(sizeof(redis_mruby));
  if (rm == NULL) {
    return NULL;
  }

  rm->mrb = mrb_open();
  if (rm->mrb == NULL) {
    free(rm);
    return NULL;
  }

  redis_mruby_init(rm);

  return rm;
}

void free_redis_mruby(redis_mruby *rm) {
  mrb_close(rm->mrb);
  free(rm);
}

char * redis_mruby_eval(redis_mruby *rm, RedisModuleString *str)
{
  const char *code;
  size_t code_len;
  mrb_value value;

  code = RedisModule_StringPtrLen(str, &code_len);
  value = mrb_load_string(rm->mrb, code);
  return (char *)mrb_string_value_ptr(rm->mrb, value);
}

mrb_value redis_mruby_command_call(mrb_state *mrb, mrb_value self) {
  return mrb_funcall(mrb, mrb_top_self(mrb), "puts", 1, mrb_str_new_cstr(mrb, "call"));
}

mrb_value redis_mruby_command_pcall(mrb_state *mrb, mrb_value self) {
  return mrb_funcall(mrb, mrb_top_self(mrb), "puts", 1, mrb_str_new_cstr(mrb, "pcall"));
}
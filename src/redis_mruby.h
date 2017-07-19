#include <stdlib.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"
#include "mruby/array.h"

#include "redismodule.h"

typedef struct redis_mruby_t {
  mrb_state *mrb;
} redis_mruby;

mrb_value redis_mruby_command_call(mrb_state *mrb, mrb_value self);
mrb_value redis_mruby_command_pcall(mrb_state *mrb, mrb_value self);

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

  return rm;
}

int redis_mruby_init(redis_mruby *rm, RedisModuleString **argv, int argc)
{
  mrb_value KEYS;
  mrb_value ARGV;

  int i; // counter for args

  int count;
  size_t count_len;
  count = atoi(RedisModule_StringPtrLen(argv[2], &count_len));

  // args: command, code, key_count, key1, key2, ..., arg1, arg2, ...
  if (argc < count + 3) {
    return NULL;
  }

  KEYS = mrb_ary_new_capa(rm->mrb, count);
  for (i = 3; i < count + 3; i++) {
    const char *arg;
    size_t arg_len;

    arg = RedisModule_StringPtrLen(argv[i], &arg_len);
    mrb_ary_push(rm->mrb, KEYS, mrb_str_new(rm->mrb, arg, arg_len));
  }
  mrb_define_global_const(rm->mrb, "KEYS", KEYS);

  ARGV = mrb_ary_new_capa(rm->mrb, argc - count - 3);
  for (; i < argc; i++) {
    const char *arg;
    size_t arg_len;

    arg = RedisModule_StringPtrLen(argv[i], &arg_len);
    mrb_ary_push(rm->mrb, ARGV, mrb_str_new(rm->mrb, arg, arg_len));
  }
  mrb_define_global_const(rm->mrb, "ARGV", ARGV);

  struct RClass *class;
  class = mrb_define_class(rm->mrb, "Redis", rm->mrb->object_class);

  mrb_define_class_method(rm->mrb, class, "call", redis_mruby_command_call,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(rm->mrb, class, "pcall", redis_mruby_command_pcall,
                          MRB_ARGS_REQ(1));
}

void free_redis_mruby(redis_mruby *rm)
{
  mrb_close(rm->mrb);
  free(rm);
}

char *redis_mruby_eval(redis_mruby *rm, const char *code)
{
  mrb_value value;

  value = mrb_load_string(rm->mrb, code);
  return (char *)mrb_string_value_ptr(rm->mrb, value);
}

mrb_value redis_mruby_command_call(mrb_state *mrb, mrb_value self)
{
  return mrb_funcall(mrb, mrb_top_self(mrb), "puts", 1,
                     mrb_str_new_cstr(mrb, "call"));
}

mrb_value redis_mruby_command_pcall(mrb_state *mrb, mrb_value self)
{
  return mrb_funcall(mrb, mrb_top_self(mrb), "puts", 1,
                     mrb_str_new_cstr(mrb, "pcall"));
}
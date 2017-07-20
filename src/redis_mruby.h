#include <stdlib.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/compile.h"
#include "mruby/string.h"

#include "redismodule.h"

typedef struct redis_mruby_t {
  mrb_state *mrb;
} redis_mruby;

mrb_value redis_mruby_command_call(mrb_state *mrb, mrb_value self);
mrb_value redis_mruby_command_pcall(mrb_state *mrb, mrb_value self);

redis_mruby *redis_mruby_new()
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

int redis_mruby_init_keys_argv(redis_mruby *rm, RedisModuleString **argv,
                               int argc, char **err)
{
  if (argc < 2) {
    *err =
        "Invalid argument(s): argument(s) count must be larger or equal than 2";
    return -1;
  }

  mrb_value KEYS;
  mrb_value ARGV;

  // argc == 2: cmd, code
  // argc >= 3: cmd, code, key_count, key1, key2, ..., arg1, arg2, ...
  if (argc == 2) {
    KEYS = mrb_ary_new_capa(rm->mrb, 0);
    ARGV = mrb_ary_new_capa(rm->mrb, 0);
  } else {
    int i;
    int num_keys;
    size_t num_keys_len;

    num_keys = atoi(RedisModule_StringPtrLen(argv[2], &num_keys_len));
    if (num_keys > (argc - 3)) {
      *err = "Invalid argument(s): number of keys can't be greater than the "
               "number of args";
      return -1;
    } else if (num_keys < 0) {
      *err = "Invalid argument(s): number of keys can't be negative";
      return -1;
    }

    KEYS = mrb_ary_new_capa(rm->mrb, num_keys);
    for (i = 3; i < num_keys + 3; i++) {
      const char *arg;
      size_t arg_len;

      arg = RedisModule_StringPtrLen(argv[i], &arg_len);
      mrb_ary_push(rm->mrb, KEYS, mrb_str_new(rm->mrb, arg, arg_len));
    }

    ARGV = mrb_ary_new_capa(rm->mrb, argc - num_keys - 3);
    for (; i < argc; i++) {
      const char *arg;
      size_t arg_len;

      arg = RedisModule_StringPtrLen(argv[i], &arg_len);
      mrb_ary_push(rm->mrb, ARGV, mrb_str_new(rm->mrb, arg, arg_len));
    }
  }

  mrb_define_global_const(rm->mrb, "KEYS", KEYS);
  mrb_define_global_const(rm->mrb, "ARGV", ARGV);

  return 0;
}

void redis_mruby_init(redis_mruby *rm)
{
  struct RClass *class;
  class = mrb_define_class(rm->mrb, "Redis", rm->mrb->object_class);

  mrb_define_class_method(rm->mrb, class, "call", redis_mruby_command_call,
                          MRB_ARGS_REQ(1));
  mrb_define_class_method(rm->mrb, class, "pcall", redis_mruby_command_pcall,
                          MRB_ARGS_REQ(1));
}

void redis_mruby_free(redis_mruby *rm)
{
  mrb_close(rm->mrb);
  free(rm);
}

int redis_mruby_eval(redis_mruby *rm, const char *code, char **result, char **err)
{
  mrb_value value;
  value = mrb_load_string(rm->mrb, code);

  if (rm->mrb->exc) {
    *err = (char *)mrb_string_value_ptr(rm->mrb, mrb_obj_value(rm->mrb->exc));
    return -1;
  } else {
    *result = (char *)mrb_string_value_ptr(rm->mrb, value);
    return 0;
  }
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
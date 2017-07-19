#include <stdlib.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"

#include "redismodule.h"

typedef struct redis_mruby_t {
  mrb_state *mrb;
} redis_mruby;

redis_mruby *new_redis_mruby()
{
  redis_mruby *self;
  self = malloc(sizeof(redis_mruby));

  if (self == NULL) {
    return NULL;
  }

  self->mrb = mrb_open();

  return self;
}

void free_redis_mruby(redis_mruby *self) {
  mrb_close(self->mrb);
  free(self);
}

char * redis_mruby_eval(redis_mruby *self, RedisModuleString *str)
{
  const char *code;
  size_t code_len;
  mrb_value value;

  code = RedisModule_StringPtrLen(str, &code_len);
  value = mrb_load_string(self->mrb, code);
  return (char *)mrb_string_value_ptr(self->mrb, value);
}
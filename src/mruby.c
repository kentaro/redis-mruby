#include <stdio.h>
#include <string.h>

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"

#include "redismodule.h"

int MRubyExec_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                           int argc) {
  if (argc != 2)
    return RedisModule_WrongArity(ctx);

  size_t code_len;
  const char *code = RedisModule_StringPtrLen(argv[1], &code_len);
  size_t result_len;
  char *result;

  mrb_state *mrb;
  mrb_value value;

  mrb = mrb_open();
  value = mrb_load_string(mrb, code);
  result = (char *)mrb_string_value_ptr(mrb, value);
  mrb_close(mrb);

  result_len = strlen(result);
  char *buf = RedisModule_PoolAlloc(ctx, result_len);
  for (size_t i = 0; i < result_len; i++)
    buf[i] = result[i];

  if (RedisModule_ReplyWithStringBuffer(ctx, buf, result_len) ==
      REDISMODULE_ERR)
    return REDISMODULE_ERR;

  return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv,
                       int argc) {
  if (RedisModule_Init(ctx, "mruby", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "mruby.exec", MRubyExec_RedisCommand,
                                "write deny-oom random fast", 1, 1,
                                1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  return REDISMODULE_OK;
}
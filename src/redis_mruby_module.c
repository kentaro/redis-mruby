#include <string.h>

#include "redis_mruby.h"
#include "redismodule.h"

int MRubyEval_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                           int argc)
{
  char *err;
  char *result;
  char *buf;
  size_t buf_len;

  const char *code;
  size_t code_len;
  code = RedisModule_StringPtrLen(argv[1], &code_len);

  redis_mruby *rm;

  rm = redis_mruby_new();
  redis_mruby_init(rm);

  if (redis_mruby_init_keys_argv(rm, argv, argc, &err) < 0)
    goto ERROR;

  if (redis_mruby_eval(rm, code, &result, &err) < 0)
    goto ERROR;

  buf_len = strlen(result);
  buf = RedisModule_PoolAlloc(ctx, buf_len);

  for (size_t i = 0; i < buf_len; i++) {
    buf[i] = result[i];
  }

  redis_mruby_free(rm);

  if (RedisModule_ReplyWithStringBuffer(ctx, buf, buf_len) == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  return REDISMODULE_OK;

ERROR:
    redis_mruby_free(rm);
    return RedisModule_ReplyWithError(ctx, err);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  if (RedisModule_Init(ctx, "mruby", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "mruby.eval", MRubyEval_RedisCommand,
                                "write deny-oom random fast", 1, 1,
                                1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  return REDISMODULE_OK;
}
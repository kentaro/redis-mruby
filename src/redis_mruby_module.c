#include <string.h>

#include "redis_mruby.h"
#include "redismodule.h"

int MRubyExec_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                           int argc)
{
  char *result;
  char *buf;
  size_t buf_len;
  redis_mruby_state *rm;

  if (argc != 2)
    return RedisModule_WrongArity(ctx);

  rm = new_redis_mruby();

  result = redis_mruby_eval(rm, argv[1]);
  buf_len = strlen(result);
  buf = RedisModule_PoolAlloc(ctx, buf_len);

  for (size_t i = 0; i < buf_len; i++) {
    buf[i] = result[i];
  }

  free_redis_mruby(rm);

  if (RedisModule_ReplyWithStringBuffer(ctx, buf, buf_len) == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  if (RedisModule_Init(ctx, "mruby", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "mruby.eval", MRubyExec_RedisCommand,
                                "write deny-oom random fast", 1, 1,
                                1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  return REDISMODULE_OK;
}
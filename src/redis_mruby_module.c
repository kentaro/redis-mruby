#include <string.h>

#include "redis_mruby.h"
#include "redismodule.h"

int redisReplyFromMrbValue(RedisModuleCtx *ctx, redis_mruby *rm, mrb_value value, char **err);

int MRubyEval_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                           int argc)
{
  char *err;
  const char *code;
  size_t code_len;
  code = RedisModule_StringPtrLen(argv[1], &code_len);

  redis_mruby *rm;
  mrb_value result;

  rm = redis_mruby_new();
  redis_mruby_init(rm);

  if (redis_mruby_init_keys_argv(rm, argv, argc, &err) < 0)
    goto ERROR;

  if (redis_mruby_eval(rm, code, &result, &err) < 0)
    goto ERROR;

  if (redisReplyFromMrbValue(ctx, rm, result, &err) < 0)
    goto ERROR;

  redis_mruby_free(rm);

  return REDISMODULE_OK;

ERROR:
    redis_mruby_free(rm);
    return RedisModule_ReplyWithError(ctx, err);
}

int redisReplyFromMrbValue(RedisModuleCtx *ctx, redis_mruby *rm, mrb_value value, char **err) {
  enum mrb_vtype type = mrb_type(value);

  if (mrb_nil_p(value)) {
    RedisModule_ReplyWithNull(ctx);
    return 0;
  }

  switch (type) {
  case MRB_TT_FLOAT: {
    RedisModule_ReplyWithDouble(ctx, (double)value.value.f);
    break;
  }
  case MRB_TT_FIXNUM: {
    RedisModule_ReplyWithLongLong(ctx, (long long)value.value.i);
    break;
  }
  case MRB_TT_ARRAY: {
    long len = (long)RARRAY_LEN(value);
    RedisModule_ReplyWithArray(ctx, len);

    for (int i = 0; i < len; i++) {
      mrb_value v = mrb_ary_ref(rm->mrb, value, i);
      redisReplyFromMrbValue(ctx, rm, v, err);
    }

    break;
  }
  default:
    RedisModule_ReplyWithNull(ctx);
  }

  return 0;
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
#include "redis_mruby.h"

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
    RedisModule_ReplyWithError(ctx, err);
    redis_mruby_free(rm);
    return REDISMODULE_ERR;
}

int redisReplyFromMrbValue(RedisModuleCtx *ctx, redis_mruby *rm, mrb_value value, char **err) {
  enum mrb_vtype type = mrb_type(value);

  if (mrb_nil_p(value)) {
    RedisModule_ReplyWithNull(ctx);
    return 0;
  }

  switch (type) {
  case MRB_TT_TRUE: {
    RedisModule_ReplyWithLongLong(ctx, 1);
    break;
  }
  case MRB_TT_FALSE: {
    RedisModule_ReplyWithNull(ctx);
    break;
  }
  case MRB_TT_FLOAT: {
    RedisModule_ReplyWithDouble(ctx, (double)value.value.f);
    break;
  }
  case MRB_TT_FIXNUM: {
    RedisModule_ReplyWithLongLong(ctx, (long long)value.value.i);
    break;
  }
  case MRB_TT_STRING: {
    RedisModuleString *rstr = RedisModule_CreateStringPrintf(ctx, "%s", mrb_str_to_cstr(rm->mrb, value));
    RedisModule_ReplyWithString(ctx, rstr);
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
  // HASH can be either `{'ok' => MSG}` or `{'err' => MSG}`
  // that are returned from `Redis.call` or `Redis.pcall` method.
  case MRB_TT_HASH: {
    mrb_value msg;

    // When the return value has a pair which has a key of "ok"
    msg = mrb_hash_get(rm->mrb, value, mrb_str_new_cstr(rm->mrb, "ok"));
    if (!mrb_nil_p(msg) && mrb_type(msg) == MRB_TT_STRING) {
      RedisModule_ReplyWithSimpleString(ctx, mrb_str_to_cstr(rm->mrb, msg));
      return 0;
    }

    // When the return value has a pair which has a key of "err"
    msg = mrb_hash_get(rm->mrb, value, mrb_str_new_cstr(rm->mrb, "err"));
    if (!mrb_nil_p(msg) && mrb_type(msg) == MRB_TT_STRING) {
      *err = mrb_str_to_cstr(rm->mrb, msg);
      return -1;
    }

    // Or, return an empty list
    RedisModule_ReplyWithArray(ctx, 0);
    break;
  }
  case MRB_TT_OBJECT: {
    RedisModule_ReplyWithNull(ctx);
    break;
  }
  case MRB_TT_EXCEPTION: {
    value = mrb_funcall(rm->mrb, value, "message", 0);
    *err = mrb_str_to_cstr(rm->mrb, value);
    return -1;
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
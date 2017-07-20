# redis-mruby

A Redis module to evaluate mruby code.

## Usage

Server:

```shell
$ redis-server --loadmodule build/redis-mruby.so
```

Client:

```shell
$ redis-cli
127.0.0.1:6379> mruby.eval "1 + 1"
"2"
```

## Command

### `mruby.eval script numkeys key1, key2, ..., arg1, arg2, ...`

Compatible to `EVAL` command that is embedded into Redis. See [https://redis.io/commands/eval](https://redis.io/commands/eval) for detail.

## Embedded Arrays

You can access `KEYS` and `ARGV` passed via cli in mruby code. 

```shell
127.0.0.1:6379> mruby.eval "[KEYS, ARGV]" 2 key1 key2 arg1 arg2
"[[\"key1\", \"key2\"], [\"arg1\", \"arg2\"]]"
```

## Author

Kentaro Kuribayashi

## License

MIT
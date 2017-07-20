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
(integer) 2
```

## Command

### `mruby.eval script numkeys key1, key2, ..., arg1, arg2, ...`

Compatible to `EVAL` command that uses lua to execute code and is embedded into Redis by default. See [https://redis.io/commands/eval](https://redis.io/commands/eval) for detail.

## Embedded Arrays

You can access `KEYS` and `ARGV` passed via cli in mruby code. 

```shell
127.0.0.1:6379> mruby.eval "[KEYS, ARGV]" 2 key1 key2 arg1 arg2
1) 1) "key1"
   2) "key2"
2) 1) "arg1"
   2) "arg2"
```

## Type Conversion

The return value from mruby code is automatically converted to some form of Redis replies listed below:

| mruby | redis |
|-----------|------------|
| `true` | integer reply with value of 1 (integer) |
| `false` | nil bulk reply |
| `Fixnum` | integer reply |
| `Float` | bulk reply with the the value converted to string expression of equivalent `double` value |
| `String` | bulk reply |
| `Array` | multi bulk reply |
| `Hash` | multi bulk reply with an empty list |
| `Object` | multi bulk reply with an empty list |
| `Exception` | error reply |

Below are several (not complete) examples of type conversion.

### Integer

```shell
127.0.0.1:6379> mruby.eval "1 + 1"
(integer) 2
```

### String

```shell
127.0.0.1:6379> mruby.eval "%q{hello}"
"hello"
```

### Array

```shell
127.0.0.1:6379> mruby.eval "[1, 2, [3, 4, [5, 6]]]"
1) (integer) 1
2) (integer) 2
3) 1) (integer) 3
   2) (integer) 4
   3) 1) (integer) 5
      2) (integer) 6
```

### Error

```shell
127.0.0.1:6379> mruby.eval "no_such_method"
(error) undefined method 'no_such_method' for main
```

## Author

Kentaro Kuribayashi

## License

MIT
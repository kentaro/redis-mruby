# redis-mruby

A Redis module to evaluate mruby code.

## SYNOPSYS

Server:

```shell
$ redis-server --loadmodule build/redis-mruby.so
```

Client:

```shell
$ redis-cli
127.0.0.1:6379> mruby.exec "1 + 1"
"2"
```

# AUTHOR

Kentaro Kuribayashi

# LICENSE

MIT
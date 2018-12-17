#

## Compile

### Fix build script:

```
 egrep -Rl offsets.c.obj | xargs sed -i 's@offsets.c.obj@offsets.c.o@g'
```

## Run

```
sudo --preserve-env=ZEPHYR_BASE zephyr/zephyr.exe
```

## TODO

  + Kconfig: log levels
  + Syslog

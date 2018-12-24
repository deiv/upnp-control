#

## Roadmap

- [ ] Discovery
  - [ ] SSDP
    - [x] Search Response
      - [x] UDP Server
        - [ ] IGMP ?
      
    - [ ] Advertisement
      - [ ] Device available
      - [ ] Device unavailable

- [ ] Description
  - [ ] Serve root xml descriptor
    - [x] HTTPD
- [ ] Control
- [ ] Eventing
- [ ] Presentation
  - [ ] Serve http doc ?¿
  
- [ ] RTOS
  - [ ] Kconfig: log levels
  - [ ] Syslog
  
## Compile

### Fix build script:

```
 egrep -Rl offsets.c.obj | xargs sed -i 's@offsets.c.obj@offsets.c.o@g'
```

## Run

```
sudo --preserve-env=ZEPHYR_BASE zephyr/zephyr.exe
```

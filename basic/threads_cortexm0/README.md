
### this is test, mainly qemu_cortex, with gdb and learning those funcs

to run OOT zephyer working base, just set up ZEPHYR_HOME

alias westActivate='theScriptToActivate'

build command:

```bash
west build -b qemu_cortex_m0 -- -DDTC_OVERLAY_FILE=app.overlay
```







## i am here to explore does where does the **IDEL**  threads exist

### Zephyr Threads on native_sim — Quick Notes

- Enable `CONFIG_THREAD_NAME=y` → GDB shows real thread names.
- Without it → all threads show as `zephyr.exe`.

#### The 3 threads

| GDB | Name | What it is |
|-----|------|------------|
| [1] | `zephyr.exe` | Native simulator's own thread (fake HW + timers). Not a Zephyr thread. |
| [3] | `main` | Zephyr main thread → runs your `main()`. |
| [4] | `idle` | Kernel's mandatory fallback thread. Always exists. |

#### Key facts

- `main` = a Zephyr thread, not C runtime entry.
- `idle` = created by kernel at boot, cannot be removed.
- `zephyr.exe` = simulator scaffolding, never renamed.
- `native_sim` → 1 Zephyr thread = 1 Linux pthread.

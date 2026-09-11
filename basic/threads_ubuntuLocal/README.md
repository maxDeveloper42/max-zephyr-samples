
### this is a example of build a target that can directly run on ubuntu

```bash
west build -b native_sim -p
```

the reason I am doing so is to use gdb to see multi-threads running.

By doing so, my host machine plays the role of a embedded system.

And i just run gdb ./build/zephyr/zephyr.exe

# Zephyr Thread Inspection Note

## Current Thread State (from `i threads`)

**Total Threads: 5** (3 application + 2 system)

| Id | Name          | Type        | State              | Notes                          |
|----|---------------|-------------|--------------------|--------------------------------|
| 1  | `zephyr.exe`  | System      | Sleeping           | Main thread, ran `main()`      |
| 4  | `idle`        | System      | Idle               | Auto-created by kernel         |
| 5  | `blink0_id`   | Application | Sleeping           | User thread from `main.c`      |
| 6  | `blink1_id`   | Application | Sleeping           | User thread from `main.c`      |
| 7  | `uart_out_id` | Application | **Running** (`*`)  | Active thread, printing output |

## Key Takeaways

- **Not many threads** — only 5 total, which is minimal and typical.
- **3 application threads**: `blink0`, `blink1`, `uart_out`.
- **2 system threads**: `main` (`zephyr.exe`) and `idle`.
- Most threads are in `__kernel_vsyscall` → **sleeping/blocked** (normal).
- The `*` marks the **currently active thread** (`uart_out_id`).
- Idle threads sleeping = **CPU not wasted** ✅

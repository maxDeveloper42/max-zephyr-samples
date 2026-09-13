/* * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>
#include <string.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

#if !DT_NODE_HAS_STATUS_OKAY(LED0_NODE)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS_OKAY(LED1_NODE)
#error "Unsupported board: led1 devicetree alias is not defined"
#endif

#define THREAD_ADDER 0
#define THREAD_PRODUCER 0
#define THREAD_REAL_BOOM 0

// a var i try to track to see with debug
int loopTracker = 0;

struct printk_data_t {
	void *fifo_reserved; /* 1st word reserved for use by fifo */
	uint32_t led;
	uint32_t cnt;
};

K_FIFO_DEFINE(printk_fifo);

struct led {
	struct gpio_dt_spec spec;
	uint8_t num;
};

static const struct led led0 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0}),
	.num = 0,
};

static const struct led led1 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED1_NODE, gpios, {0}),
	.num = 1,
};

void blink(const struct led *led, uint32_t sleep_ms, uint32_t id)
{
	const struct gpio_dt_spec *spec = &led->spec;
	int cnt = 0;
	int ret;

	if (!device_is_ready(spec->port)) {
		printk("Error: %s device is not ready\n", spec->port->name);
		return;
	}

	ret = gpio_pin_configure_dt(spec, GPIO_OUTPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure pin %d (LED '%d')\n",
			ret, spec->pin, led->num);
		return;
	}

	while (1) {
		gpio_pin_set(spec->port, spec->pin, cnt % 2);

		struct printk_data_t tx_data = { .led = id, .cnt = cnt };

		size_t size = sizeof(struct printk_data_t);
		char *mem_ptr = k_malloc(size);
		__ASSERT_NO_MSG(mem_ptr != 0);

		memcpy(mem_ptr, &tx_data, size);

		k_fifo_put(&printk_fifo, mem_ptr);

		k_msleep(sleep_ms);
		cnt++;
	}
}

void blink0(void)
{
	blink(&led0, 100, 0);
}

void blink1(void)
{
	blink(&led1, 1000, 1);
}

void uart_out(void)
{
	while (1) {
		struct printk_data_t *rx_data = k_fifo_get(&printk_fifo,
							   K_FOREVER);
		printk("Toggled led%d; counter=%d\n",
		       rx_data->led, rx_data->cnt);

		// printk("this is a line created by Max. \n");

		if(rx_data->cnt%97 == 0){
			printk("=============the number can be devided by 97========================, %i\n",loopTracker);
			loopTracker++;
		}

		k_free(rx_data);
		
	}
}


K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL,
		PRIORITY, 0, 0);
K_THREAD_DEFINE(blink1_id, STACKSIZE, blink1, NULL, NULL, NULL,
		PRIORITY, 0, 0);
K_THREAD_DEFINE(uart_out_id, STACKSIZE, uart_out, NULL, NULL, NULL,
		PRIORITY, 0, 0);









#if THREAD_ADDER

// global for dynamic thread
K_THREAD_STACK_DEFINE(dyn_stack, 1024);
struct k_thread dyn_tcb;

void dyn_thread_func(void *p1, void *p2, void *p3)
{
	while(1) {
		printk("dynamic thread running, with number %i\n", loopTracker );
		k_msleep(1000);
	}
}



void adding_thread(){
	

	k_thread_create(&dyn_tcb, dyn_stack, K_THREAD_STACK_SIZEOF(dyn_stack),
			dyn_thread_func, NULL,NULL,NULL,
			PRIORITY, 0, K_NO_WAIT);

	while(1){
		printk("this is the thread added by max.\n");
		loopTracker++;

		/*
		if(!created){
			k_thread_create(&dyn_tcb, dyn_stack, K_THREAD_STACK_SIZEOF(dyn_stack),
					dyn_thread_func, NULL,NULL,NULL,
					PRIORITY, 0, 0);
			created = true;
		}
		*/

		/*
		// this can not been achieved bcs we shall not use the same dyn_tcb to create new thread
		if(THREAD_PRODUCER){
			k_thread_create(&dyn_tcb, dyn_stack, K_THREAD_STACK_SIZEOF(dyn_stack),
					dyn_thread_func, NULL,NULL,NULL,
					PRIORITY, 0, K_NO_WAIT);
			//created = true;
		}
		*/

		k_msleep(1000);
	}
}

K_THREAD_DEFINE(max_thread_id, STACKSIZE, adding_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

// here is a comment

#endif












#if THREAD_PRODUCER

#define MAX_DYN_THREADS 88
K_THREAD_STACK_ARRAY_DEFINE(dyn_d_stacks, MAX_DYN_THREADS, 1024);
static struct k_thread dyn_d_tcb[MAX_DYN_THREADS];
static int dyn_d_count = 0;


void dynamically_adding_thread(void)
{
    while (1) {
        if (dyn_d_count < MAX_DYN_THREADS) {
            k_thread_create(&dyn_d_tcb[dyn_d_count],
                            dyn_d_stacks[dyn_d_count],
                            K_THREAD_STACK_SIZEOF(dyn_d_stacks[0]),dynamically_adding_thread, NULL, NULL, NULL,PRIORITY, 0, K_NO_WAIT);
            dyn_d_count++;
        } else {
            printk("Max dynamic threads reached\n");
        }

        printk("this is the thread added by max.\n");
        loopTracker++;
        k_msleep(1000);
    }
}


K_THREAD_DEFINE(dynamically_id, STACKSIZE, dynamically_adding_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

#endif










#if THREAD_REAL_BOOM
 
int boom_threads_id = 0;


/*
You can't create a "real thread boom" with the `west` build system because Zephyr's thread stacks and thread control blocks must be **statically defined at file scope during compile time**, and **cannot be declared dynamically inside a function**.

In other words:

- `K_THREAD_STACK_DEFINE` / `K_THREAD_DEFINE` expand to global variables placed in special linker sections — so they must be at **file scope**, not inside a function.
- To spawn threads at runtime you need `k_thread_create`, but the stack memory and `struct k_thread` still have to be **pre-allocated** (usually as file-scope arrays, or via `k_malloc`).
- `west` itself is not the limitation — the limitation comes from **C scope rules** and **Zephyr's kernel API**.
*/

void threads_boom(){
	while(1){
		printk("this is the threads booomer func, with the boom_threads_id of %i\n", boom_threads_id);
		boom_threads_id++;
	
		// The real problem: K_THREAD_DEFINE cannot be called inside a function
		// K_THREAD_DEFINE(void, STACKSIZE, threads_boom, NULL, NULL, NULL, PRIORITY, 0, 0);

		 //K_THREAD_STACK_DEFINE is also a file-scope (static, linker-section) macro — it cannot be used inside a function.		
		// K_THREAD_STACK_DEFINE(boom_threads_id, 1024);

		k_msleep(1000);
	}
}


K_THREAD_DEFINE(thread_boomer_id, STACKSIZE, threads_boom, NULL, NULL, NULL, PRIORITY, 0, 0);




#endif


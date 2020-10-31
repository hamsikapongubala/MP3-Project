#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "acceleration.h"
#include "adc.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "ssp2.h"
#include "uart_lab.h"

static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
static QueueHandle_t song_name_q;
static QueueHandle_t mp3_data_q;

typedef char song_data_t[512];
typedef char song_name_t[32];

// CLI sends the song name
void CLI_simulator() {
  song_name_t song = {};
  strncpy(song, "name.mp3", sizeof(song) - 1);
  if (xQueueSend(song_name_q, song, 0)) { // with &?
    puts("Sent songname\n");
  } else {
    puts("Couldn't send songname to queue\n");
  }
  vTaskSuspend(NULL);
}

// open file
void open_mp3_file(char *file_name) {
  // const char *filename = "file.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, file_name, (FA_READ)); // Check what file permissions
  song_data_t buffer = {};                              // init to 0
  if (FR_OK == result) {

    while (FR_OK == f_read(&file, buffer, strlen(buffer), &bytes_written)) {
      if (xQueueSend(mp3_data_q, buffer, portMAX_DELAY)) {
        ;
      } else {
        puts("mp3_data_q send error\n");
      }
      memset(&buffer[0], 0, sizeof(buffer));
    }
    printf("ERROR: Failed to read data to file\n");
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", buffer);
  }
}

// reads mp3 file and receives song name from CLI
void mp3_file_reader_task() {
  song_name_t song_name = {};

  while (1) {
    if (xQueueReceive(song_name_q, song_name, 5000)) {
      open_mp3_file(song_name);
    } else {
      puts("song_name_q receive error\n");
    }
  }
}

void mp3_file_decoder(song_data_t song_data) {
  for (int i = 0; i < sizeof(song_data_t); i++) {
    putchar(song_data[i]);
  }
}

// plays file
void play_file_task() {
  song_data_t song_data = {};
  while (1) {
    memset(&song_data[0], 0, sizeof(song_data_t));
    if (xQueueReceive(mp3_data_q, &song_data[0], portMAX_DELAY)) {
      // play pause function
      mp3_file_decoder(song_data);
    } else {
      puts("mp3_data_q receive error\n");
    }
  }
}

app_cli_status_e cli__task_control(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                   app_cli__print_string_function cli_output) {
  sl_string_t s = user_input_minus_command_name;

  // If the user types 'taskcontrol suspend led0' then we need to suspend a task with the name of 'led0'
  // In this case, the user_input_minus_command_name will be set to 'suspend led0' with the command-name removed
  if (sl_string__begins_with_ignore_case(s, "suspend")) {

    // TODO: Use sl_string API to remove the first word, such that variable 's' will equal to 'led0'
    char led0[16];
    sl_string__scanf("%*s %16s", led0);
    cli_output(NULL, s);

    // Now try to query the tasks with the name 'led0'
    TaskHandle_t task_handle = xTaskGetHandle(s);

    if (NULL == task_handle) {
      // note: we cannot use 'sl_string__printf("Failed to find %s", s);' because that would print existing string
      // itself
      sl_string__insert_at(s, 0, "Could not find a task with name:");
      cli_output(NULL, s);
    } else {
      vTaskSuspend(task_handle);
    }

  } else if (sl_string__begins_with_ignore_case(s, "resume")) {

    TaskHandle_t task_handle = xTaskGetHandle(s);
    char led0[16];
    sl_string__scanf("%*s %16s", led0);
    cli_output(NULL, s);

    if (task_handle == NULL) {
      sl_string__insert_at(s, 0, "Could not find a task with name");
      cli_output(NULL, s);
    } else {
      vTaskResume(task_handle);
    }
  } else {
    cli_output(NULL, "Did you mean to say suspend or resume?\n");
  }

  return APP_CLI_STATUS__SUCCESS;
}

int main(void) {
  setvbuf(stdout, 0, _IONBF, 0);

  mp3_data_q = xQueueCreate(2, sizeof(song_data_t));
  song_name_q = xQueueCreate(1, sizeof(song_data_t));

  xTaskCreate(CLI_simulator, "CLI", 1, NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_file_reader_task, "reader", 1, NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(play_file_task, "player", 1, NULL, PRIORITY_HIGH, NULL);

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
static void create_blinky_tasks(void) {
  /**
   * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
   */
#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
#else
  const bool run_1000hz = true;
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit CPU
  periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz task
  UNUSED(blink_task);
#endif
}

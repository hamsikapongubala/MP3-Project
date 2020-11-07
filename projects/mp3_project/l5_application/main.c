#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "ff.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
#include "uart_lab.h"

static QueueHandle_t song_name_q;
static QueueHandle_t mp3_data_q;

typedef char song_data_t[512];
typedef char song_name_t[32];

void mp3_reader_task() {
  song_name_t file_name = {};
  song_data_t buffer = {};
  FIL file;
  UINT bytes_written;

  while (1) {

    xQueueReceive(song_name_q, &file_name[0], portMAX_DELAY);
    printf("Received song to play: %s\n", file_name);

    FRESULT result = f_open(&file, file_name, (FA_READ));

    if (FR_OK == result) {

      f_read(&file, buffer, sizeof(buffer), &bytes_written);

      while (bytes_written != 0) {
        f_read(&file, buffer, sizeof(buffer), &bytes_written);
        xQueueSend(mp3_data_q, &buffer[0], portMAX_DELAY);
        memset(&buffer[0], 0, sizeof(buffer));
      }

      f_close(&file);
    }
  }
}

void mp3_file_decoder(song_data_t song_data) {
  for (int i = 0; i < sizeof(song_data_t); i++) {
    putchar(song_data[i]);
  }
}

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

app_cli_status_e cli__mp3_play(app_cli__argument_t argument,
                               sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;

  if (sl_string__begins_with_ignore_case(s, "play")) {

    sl_string__erase_first_word(s, ' ');

    // s will be our string name now
    if (xQueueSend(song_name_q, s, 0)) {
      puts("Sent songname\n");
    } else {
      puts("Couldn't send songname to queue\n");
    }

  } else {
    cli_output(NULL, "Did you mean play?\n");
  }
  return APP_CLI_STATUS__SUCCESS;
}

int main(void) {
  setvbuf(stdout, 0, _IONBF, 0);

  mp3_data_q = xQueueCreate(2, sizeof(song_data_t));
  song_name_q = xQueueCreate(1, sizeof(song_data_t));

  xTaskCreate(mp3_reader_task, "reader", 4096, NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(play_file_task, "player", 4096, NULL, PRIORITY_HIGH, NULL);

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler
                         // runs out of memory and fails

  return 0;
}

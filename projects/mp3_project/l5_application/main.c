#include <stdio.h>

#include "FreeRTOS.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "ff.h"
#include "mp3_decoder.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
#include "task.h"
#include "uart.h"
#include "lcd.h"

QueueHandle_t song_name_q;
static QueueHandle_t mp3_data_q;

typedef char song_data_t[128];
typedef char song_name_t[32];

// open file
void open_mp3_file(char *file_name) {
  song_data_t buffer = {};
  FIL file;
  UINT bytes_written;

  FRESULT result = f_open(&file, file_name, (FA_READ));

  if (FR_OK == result) {

    f_read(&file, buffer, sizeof(buffer), &bytes_written);
    while (bytes_written != 0) {
      f_read(&file, buffer, sizeof(buffer), &bytes_written);
      xQueueSend(mp3_data_q, buffer, portMAX_DELAY);
      memset(&buffer[0], 0, sizeof(buffer));
    }
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", file_name);
  }
}

// reads mp3 file and receives song name from CLI
void mp3_file_reader_task() {
  song_name_t song_name = {};

  while (1) {
    if (xQueueReceive(song_name_q, song_name, portMAX_DELAY)) {
      open_mp3_file(song_name);
    }
  }
}

void mp3_file_decoder(song_data_t song_data) {
  for (int i = 0; i < sizeof(song_data_t); i++) {
    // printf("%s", song_data[i]);
    putchar(song_data[i]);
  }
}

// plays file
void play_file_task() {
  song_data_t song_data = {};
  while (1) {
    memset(&song_data[0], 0, sizeof(song_data_t));
    if (xQueueReceive(mp3_data_q, song_data, portMAX_DELAY)) {
      for (int i = 0; i < 128; i++) {
        while (!DREQ_status()) {
          ;
        }
        send_mp3Data(song_data[i]);
      }
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
      // printf("Sent songname\n");
    } else {
      printf("Couldn't send songname to queue\n");
    }

  } else {
    cli_output(NULL, "Did you mean play?\n");
  }
  return APP_CLI_STATUS__SUCCESS;
}

int main(void) {
  mp3_decoder_setup();
  lcd_init();
  LCD_print_char("H");

  setvbuf(stdout, 0, _IONBF, 0);

  mp3_data_q = xQueueCreate(2, sizeof(song_data_t));
  song_name_q = xQueueCreate(1, sizeof(song_data_t));

  // xTaskCreate(CLI_simulator, "CLI", 1024, NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_file_reader_task, "reader", 2048 / sizeof(void *), NULL,
              PRIORITY_MEDIUM, NULL);
  xTaskCreate(play_file_task, "player", 8192, NULL, PRIORITY_HIGH, NULL);
  sj2_cli__init();
  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler
                         // runs out of memory and fails

  return 0;
}
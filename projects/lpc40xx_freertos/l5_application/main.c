#include <stdio.h>

#include "FreeRTOS.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "lcd.h"
#include "lpc_peripherals.h"
#include "mp3_decoder.h"
#include "queue.h"
#include "sj2_cli.h"
#include "song_list.h"
#include "task.h"

TaskHandle_t reader_handle;

QueueHandle_t song_name_q;
static QueueHandle_t mp3_data_q;

SemaphoreHandle_t select_sem;
volatile int current_playing;

volatile bool pause = false;
volatile bool next = false;
volatile bool previous = false;

bool playlist = false;
bool main_menu = true;
bool volume_menu = false;
bool settings_menu = false;
bool trebble_bass_menu = false;
bool now_playing_menu = false;
bool new_song_list = false;

int page = 0;

int volume_count = 3;
int bass_count = 3;
int treble_count = 5;

gpio_s sel, vol_up, vol_down, prev, next_gpio, back, down_bass, down_treble, up_bass, up_treble, song_page;

typedef char song_data_t[128];
typedef char song_name_t[32];

void open_mp3_file(char *file_name) {
  song_data_t buffer = {};
  FIL file;
  UINT bytes_written;

  FRESULT result = f_open(&file, file_name, (FA_READ));

  if (FR_OK == result) {
    f_read(&file, buffer, sizeof(buffer), &bytes_written);
    while (bytes_written != 0) {
      if (pause) {
        // LCD_print_string("Now Playing: ");
      } else {
        f_read(&file, buffer, sizeof(buffer), &bytes_written);
        xQueueSend(mp3_data_q, buffer, portMAX_DELAY);
      }

      if (next) {
        next = false;
        break;
      }

      if (previous) {
        previous = false;
        break;
      }
    }

    f_close(&file);

  } else {
    fprintf(stderr, "ERROR: Failed to open: %s\n", file_name);
  }
}

typedef struct {
  char name[32];
  char artist[32];
} current_song;

void read_song_info(char *file_name) {
  char song_info[128];
  FIL file;
  UINT bytes_written;

  FRESULT result = f_open(&file, file_name, (FA_READ));

  if (FR_OK == result) {
    f_lseek(&file, f_size(&file) - (sizeof(char) * 128));
    f_read(&file, song_info, sizeof(song_info), &bytes_written);
    print_song_info(song_info);
    f_lseek(&file, 0);
    f_close(&file);
  } else {
    fprintf(stderr, "Error reading song info\n");
  }
}

void print_song_info(char *song_info) {
  current_song info = {0};
  int count_name = 0;
  int count_artist = 0;
  for (int i = 0; i < 128; i++) {
    if ((((int)(song_info[i]) > 47) && ((int)(song_info[i]) < 58)) ||  // number
        (((int)(song_info[i]) > 64) && ((int)(song_info[i]) < 91)) ||  // ALPHABET
        (((int)(song_info[i]) > 96) && ((int)(song_info[i]) < 123)) || // alphabet
        ((int)(song_info[i])) == 32) {

      if (i > 2 && i < 33) {
        info.name[count_name++] = (int)(song_info[i]);
      } else if (i > 32 && i < 63) {
        info.artist[count_artist++] = (int)(song_info[i]);
      }
    }
  }
  LCD_display_clear();
  lcd_playlist(0, 0);
  // LCD_print_string("Now playing: ");
  LCD_print_string(info.name);
  lcd_playlist(1, 0);
  LCD_print_string("By: ");
  LCD_print_string(info.artist);
}

void mp3_file_reader_task() {
  song_name_t song_name = {};
  while (1) {
    if (xQueueReceive(song_name_q, song_name, portMAX_DELAY)) {
      fprintf(stderr, "Received Song %s\n", song_name);
      read_song_info(song_name); // this line was added
      open_mp3_file(song_name);
    }
  }
}

void play_file_task() {
  song_data_t song_data = {};
  while (1) {
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

void play_pause_ISR() {
  if (pause) {
    fprintf(stderr, "pause set to false\n");
    pause = false;
  } else {
    pause = true;
    fprintf(stderr, "pause set to true\n");
  }
}

uint8_t max_volume = 0x00;
uint8_t min_volume = 0xFA;
uint16_t current_volume = 0x20;

void volume_up() {
  if (current_volume <= max_volume) {
    fprintf(stderr, "Volume at highest\n");
  } else {
    current_volume = current_volume - 0x10;
    write_register(0x0B, current_volume, current_volume);
    fprintf(stderr, "Increasing volume.\n");
    volume_count++;
  }
}
void volume_down() {
  if (current_volume >= min_volume) {
    fprintf(stderr, "Volume at lowest\n");
  } else if (volume_count == 0) {
    fprintf(stderr, "Volume at lowest\n");
  } else {
    current_volume = current_volume + 0x10;
    write_register(0x0B, current_volume, current_volume);
    fprintf(stderr, "Lowering volume.\n");
    volume_count--;
  }
}

uint8_t max_treble = 0b10000110;
uint8_t min_treble = 0b10010110;
uint8_t min_bass = 0xF6;
uint8_t max_bass = 0x07;
uint8_t initial_bass = 0x06;
uint8_t initial_treble = 0b00000110;

void treble_up() {
  if (initial_treble >= max_treble) {
    ;
  } else {
    initial_treble = initial_treble - 0b1000;
    write_register(0x0B, initial_treble, initial_treble);
    fprintf(stderr, "Treble Increased \n");
    treble_count++;
  }
}

void treble_down() {
  if (initial_treble == min_treble) {
    ;
  } else if (treble_count == 0) {
    ;
  } else {
    initial_treble = initial_treble + 0b1000;
    write_register(0x0B, initial_treble, initial_treble);
    fprintf(stderr, "Treble Decreased \n");
    treble_count--;
  }
}

void bass_up() {
  if (initial_bass <= max_bass) {
    ;
  } else {
    initial_bass = initial_bass - 0x20;
    write_register(0x0B, initial_bass, initial_bass);
    fprintf(stderr, "Bass Increased \n");
    bass_count++;
  }
}

void bass_down() {
  if (initial_bass == min_bass) {
    ;
  } else if (bass_count == 0) {
    ;
  } else {
    initial_bass = initial_bass + 0x20;
    write_register(0x0B, initial_bass, initial_bass);
    fprintf(stderr, "Bass Decreased \n");
    bass_count--;
  }
}

void cursor_isr() {
  // int col = get_col();
  int row = get_row();
  if (row == 1) {
    lcd_playlist(0, 0);
  } else {
    lcd_playlist(1, 0);
    // fprintf(stderr, "Col: %d", col);
  }
}

void build_menu(int start) {
  LCD_display_clear();
  int row = 0;
  fprintf(stderr, "Build before for-loop\n");
  for (int i = start; i < (start + 2); i++) {
    char *temp_str = song_list__get_name_for_item(i);
    fprintf(stderr, "Temp str: %s\n", temp_str);
    lcd_playlist(row, 0);
    LCD_print_string(temp_str);
    LCD_print_string(" ");
    row++;
  }
}
void build_main_menu() {
  LCD_display_clear();
  lcd_playlist(0, 0);
  LCD_print_string("playlist");
  lcd_playlist(1, 0);
  LCD_print_string("settings");
}

void build_setting_menu() {
  LCD_display_clear();
  lcd_playlist(0, 0);
  LCD_print_string("Volume");
  lcd_playlist(1, 0);
  LCD_print_string("Treble and Bass");
}
void build_vol_menu() {
  char c[10];
  LCD_display_clear();
  lcd_playlist(0, 0);
  LCD_print_string("Volume :");
  sprintf(c, "%d", volume_count);
  LCD_print_string(c);
}
void build_trebble_bass_menu() {
  char t[10];
  char b[10];
  LCD_display_clear();
  lcd_playlist(0, 0);
  LCD_print_string("Treble: ");
  sprintf(t, "%d", treble_count);
  LCD_print_string(t);
  lcd_playlist(1, 1);
  LCD_print_string("Bass: ");
  sprintf(b, "%d", bass_count);
  LCD_print_string(b);
}

void select() {
  if (main_menu) {
    if (get_row() == 0) {
      playlist = true;
      build_menu(0);
      main_menu = false;
    } else {
      build_setting_menu();
      settings_menu = true;
      main_menu = false;
    }
  } else if (playlist) {
    if (song_list__get_item_count() > 0) {
      int row = get_row();
      current_playing = row;
      char *s = song_list__get_name_for_item(row);
      now_playing_menu = true;
      LCD_display_clear();
      // LCD_print_string("Now Playing: ");
      // LCD_print_string(s);
      // lcd_playlist(1, 1);
      // LCD_print_string("By: ");
      xQueueSendFromISR(song_name_q, s, 0);
    }
  } else if (new_song_list) {
    if (song_list__get_item_count() > 0) {
      int row = get_row() + 2;
      current_playing = row;
      char *s = song_list__get_name_for_item(row);
      now_playing_menu = true;
      LCD_display_clear();
      // LCD_print_string("Now Playing: ");
      // LCD_print_string(s);
      // lcd_playlist(1, 1);
      // LCD_print_string("By: ");
      xQueueSendFromISR(song_name_q, s, 0);
    }
  } else if (settings_menu) {
    if (get_row() == 0) {
      settings_menu = false;
      volume_menu = true;
      build_vol_menu();
    } else {
      settings_menu = false;
      trebble_bass_menu = true;
      build_trebble_bass_menu();
    }
  }
}

void new_song_page(void) {
  if (playlist) {
    new_song_list = true;
    playlist = false;
    build_menu(2);
  }
}

void select_isr(void) {
  select();
  fprintf(stderr, "select isr\n");
}
void up_isr() {
  if (volume_menu) {
    build_vol_menu();
    volume_up();
  } else if (trebble_bass_menu) {
    if (get_row() == 0) {
      treble_up();
      build_trebble_bass_menu();
      lcd_playlist(0, 0);
    } else {
      bass_up();
      build_trebble_bass_menu();
      lcd_playlist(1, 0);
    }
  } else {
    volume_up();
  }
}

void down_isr() {
  if (volume_menu) {
    build_vol_menu();
    volume_down();
  } else if (trebble_bass_menu) {
    if (get_row() == 0) {
      treble_down();
      build_trebble_bass_menu();
      lcd_playlist(0, 0);
    } else {
      bass_down();
      build_trebble_bass_menu();
      lcd_playlist(1, 0);
    }
  } else {
    volume_down();
  }
}

void back_isr() {
  if (volume_menu) {
    volume_menu = false;
    settings_menu = true;
    build_setting_menu();
  } else if (trebble_bass_menu) {
    trebble_bass_menu = false;
    settings_menu = true;
    build_setting_menu();
  } else if (settings_menu) {
    settings_menu = false;
    main_menu = true;
    build_main_menu();
  } else if (now_playing_menu) {
    now_playing_menu = false;
    playlist = true;
    build_menu(0);
  } else if (playlist) {
    playlist = false;
    main_menu = true;
    build_main_menu();
  } else if (new_song_list) {
    playlist = true;
    new_song_list = false;
    build_menu(0);
  }
}

void prev_song_isr() {
  fprintf(stderr, "prev song isr\n");
  if (current_playing != 0) {
    char *s = song_list__get_name_for_item(current_playing - 1);
    current_playing--;
    fprintf(stderr, "prev current playing: %d\n", current_playing);
    previous = true;
    xQueueSendFromISR(song_name_q, s, 0);
  }
}

void next_song_isr(void) {
  fprintf(stderr, "next song isr\n");
  if (current_playing != song_list__get_item_count() - 1) {
    char *s = song_list__get_name_for_item(current_playing + 1);
    current_playing++;
    fprintf(stderr, "next current playing: %d\n", current_playing);
    next = true;
    xQueueSendFromISR(song_name_q, s, 0);
  }
}

void button_init() {
  LPC_GPIO0->DIR &= ~(1U << 30);
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, cursor_isr); // cursor_isr

  LPC_GPIO0->DIR &= ~(1U << 29);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, play_pause_ISR);

  sel = gpio__construct_with_function(GPIO__PORT_0, 25, 0);
  gpio__set_as_input(sel);
  gpio0__attach_interrupt(25, GPIO_INTR__FALLING_EDGE, select_isr); // select_isr

  vol_up = gpio__construct_with_function(GPIO__PORT_0, 26, 0);
  LPC_GPIO0->DIR &= ~(1U << 26);
  gpio0__attach_interrupt(26, GPIO_INTR__FALLING_EDGE, prev_song_isr); // prev_song_isr

  vol_down = gpio__construct_with_function(GPIO__PORT_0, 0, 0);
  LPC_GPIO0->DIR &= ~(1U << 0);
  gpio0__attach_interrupt(0, GPIO_INTR__FALLING_EDGE, next_song_isr); // next_song_isr

  song_page = gpio__construct_with_function(GPIO__PORT_0, 10, 10);
  gpio__set_as_input(song_page);
  gpio0__attach_interrupt(10, GPIO_INTR__FALLING_EDGE, new_song_page);

  prev = gpio__construct_with_function(GPIO__PORT_0, 1, 0);
  gpio__set_as_input(prev);
  gpio0__attach_interrupt(1, GPIO_INTR__FALLING_EDGE, up_isr); // up_isr

  next_gpio = gpio__construct_with_function(GPIO__PORT_0, 22, 0);
  gpio__set_as_input(next_gpio);
  gpio0__attach_interrupt(22, GPIO_INTR__FALLING_EDGE, down_isr);

  back = gpio__construct_with_function(GPIO__PORT_0, 11, 0);
  gpio__set_as_input(next_gpio);
  gpio0__attach_interrupt(11, GPIO_INTR__FALLING_EDGE, back_isr);

  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, NULL);
  // NVIC_EnableIRQ(GPIO_IRQn);
}

int main(void) {
  button_init();
  mp3_decoder_setup();

  setvbuf(stdout, 0, _IONBF, 0);

  mp3_data_q = xQueueCreate(1, sizeof(song_data_t));
  song_name_q = xQueueCreate(1, sizeof(song_data_t));
  select_sem = xSemaphoreCreateBinary();

  xTaskCreate(mp3_file_reader_task, "reader", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, &reader_handle);
  xTaskCreate(play_file_task, "player", 8192, NULL, PRIORITY_MEDIUM, NULL);
  sj2_cli__init();

  song_list__populate();
  for (size_t song_number = 0; song_number < song_list__get_item_count(); song_number++) {
    printf("Song %2d: %s\n", (1 + song_number), song_list__get_name_for_item(song_number));
  }

  lcd_init();
  LCD_display_clear();
  build_main_menu();

  vTaskStartScheduler();

  return 0;
}

app_cli_status_e cli__mp3_play(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;

  if (sl_string__begins_with_ignore_case(s, "play")) {

    sl_string__erase_first_word(s, ' ');

    if (xQueueSend(song_name_q, s, 0)) {

    } else {
      printf("Couldn't send songname to queue\n");
    }

  } else {
    cli_output(NULL, "Did you mean play?\n");
  }
  return APP_CLI_STATUS__SUCCESS;
}
#include "stdio.h"
#include <string.h>

#include "song_list.h"

#include "ff.h"

static song_memory_t list_of_songs[32];
static size_t number_of_songs;

static void song_list__handle_filename(const char *filename) {
  // This will not work for cases like "file.mp3.zip"
  if (NULL != strstr(filename, ".mp3")) {
    // printf("Filename: %s\n", filename);

    // Dangerous function: If filename is > 128 chars, then it will copy extra
    // bytes leading to memory corruption strcpy(list_of_songs[number_of_songs],
    // filename);

    // Better: But strncpy() does not guarantee to copy null char if max length
    // encountered So we can manually subtract 1 to reserve as NULL char
    strncpy(list_of_songs[number_of_songs], filename,
            sizeof(song_memory_t) - 1);

    // Best: Compensates for the null, so if 128 char filename, then it copies
    // 127 chars, AND the NULL char snprintf(list_of_songs[number_of_songs],
    // sizeof(song_memory_t), "%.149s", filename);

    ++number_of_songs;
    // or
    // number_of_songs++;
  }
}

void song_list__populate(void) {
  FRESULT res;
  FILINFO file_info;
  char root_path[128];

  DIR dir;
  res = f_opendir(&dir, root_path);
  // fprintf(stderr, "res after open: %d\n", res);

  if (res == FR_OK) {
    res = f_readdir(&dir, &file_info);
    // fprintf(stderr, "res after read1: %d\n", res);

    for (;;) {
      res = f_readdir(&dir, &file_info); /* Read a directory item */
      // fprintf(stderr, "res after read2: %d\n", res);

      if (res != FR_OK || file_info.fname[0] == 0) {
        // fprintf(stderr, "file info1: %d\n", file_info.fname[0]);
        break; /* Break on error or end of dir */
      }

      if (file_info.fattrib & AM_DIR) {
        // fprintf(stderr, "file info2: %d\n", file_info.fname[0]);
        /* Skip nested directories, only focus on MP3 songs at the root */
      } else { /* It is a file. */
        song_list__handle_filename(file_info.fname);
        // fprintf(stderr, "file info3: %d\n", file_info.fname[0]);
      }
    }
    f_closedir(&dir);
  }
}

size_t song_list__get_item_count(void) { return number_of_songs; }

const char *song_list__get_name_for_item(size_t item_number) {
  const char *return_pointer = "";

  if (item_number >= number_of_songs) {
    return_pointer = "";
  } else {
    return_pointer = list_of_songs[item_number];
  }

  return return_pointer;
}
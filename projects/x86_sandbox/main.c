#include <queue.h>
#include <stdio.h>

static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

switch_e get_switch_input_from_switch0() {
  switch_e value;
  value.switch__on = 1;
  value.switch__off = 0;
  return value;
}

// TODO: Create this task at PRIORITY_LOW
void producer(void *p) {
  while (1) {
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    const switch_e switch_value = get_switch_input_from_switch0();

    // TODO: Print a message before xQueueSend()
    printf("Before xQueueSend()\n");
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    xQueueSend(switch_queue, &switch_value, 0);
    // TODO: Print a message after xQueueSend()
    printf("After xQueueSend()\n");

    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) {
  switch_e switch_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    printf("Before xQueueReceive\n");
    xQueueReceive(switch_queue, &switch_value, portMAX_DELAY);
    // TODO: Print a message after xQueueReceive()
    printf("After xQueueReceive\n");
  }
}

int main(void) {
  // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(2, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)

  // TODO: Create your tasks
  xTaskCreate(producer, "producer", 1024 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer, "consumer", 1024 / sizeof(void *), NULL, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
}

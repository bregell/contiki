/*
 * Copyright (c) 2015, Johan Bregell, i3tex
 */

/**
 * \file
 *      Temperature Resource Openmote
 * \author
 *      Johan Bregell <johan@bregell.se>
 */

#include <stdlib.h>
#include <string.h>
#include "dev/leds.h"
#include "rest-engine.h"
#include "dev/sht21.h"
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 800; ++k) {
    j = k > 400 ? 800 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm ("nop");
    }
    leds_off(l);
    for(i = 0; i < 400 - j; ++i) {
      asm ("nop");
    }
  }
}

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_temp,
         "title=\"Temperature\";rt=\"temperature\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  uint16_t sht21_raw_temp = temp_sensor.value(SHT21_TEMP_VAL);
  double sht21_temp = sht21_convert_temperature(sht21_raw_temp);
  unsigned int accept = -1;

  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN){
    REST.set_header_content_type(request, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%.2f:%d", sht21_temp, (int)sht21_temp);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
    fade(LEDS_GREEN);
  } else if(accept == REST.type.APPLICATION_JSON){
    REST.set_header_content_type(request, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'temp':{'float':%.2f, 'int':%d}}", sht21_temp, (int)sht21_temp);
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
    fade(LEDS_GREEN);
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
    fade(LEDS_RED);
  }
}

// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __ESP_LOG_H__
#define __ESP_LOG_H__

#include <stdint.h>
#include <stdarg.h>
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Logging library
 *
 * Log library has two ways of managing log verbosity: compile time, set via
 * menuconfig, and runtime, using esp_log_set_level function.
 *
 * At compile time, filtering is done using CONFIG_LOG_DEFAULT_LEVEL macro, set via
 * menuconfig. All logging statments for levels higher than CONFIG_LOG_DEFAULT_LEVEL
 * will be removed by the preprocessor.
 *
 * At run time, all logs below CONFIG_LOG_DEFAULT_LEVEL are enabled by default.
 * esp_log_set_level function may be used to set logging level per module.
 * Modules are identified by their tags, which are human-readable ASCII
 * zero-terminated strings.
 *
 * How to use this library:
 *
 * In each C file which uses logging functionality, define TAG variable like this:
 *
 *      static const char* TAG = "MyModule";
 *
 * then use one of logging macros to produce output, e.g:
 *
 *      ESP_LOGW(TAG, "Baud rate error %.1f%%. Requested: %d baud, actual: %d baud", error * 100, baud_req, baud_real);
 *
 * Several macros are available for different verbosity levels:
 *
 *      ESP_LOGE — error
 *      ESP_LOGW — warning
 *      ESP_LOGI — info
 *      ESP_LOGD - debug
 *      ESP_LOGV - verbose
 *
 * Additionally there is an _EARLY_ variant for each of these macros (e.g. ESP_EARLY_LOGE).
 * These variants can run in startup code, before heap allocator and syscalls
 * have been initialized.
 * When compiling bootloader, normal ESP_LOGx macros fall back to the same implementation
 * as ESP_EARLY_LOGx macros. So the only place where ESP_EARLY_LOGx have to be used explicitly
 * is the early startup code, such as heap allocator initialization code.
 *
 * (Note that such distinction would not have been necessary if we would have an
 * ets_vprintf function in the ROM. Then it would be possible to switch implementation
 * from _EARLY version to normal version on the fly. Unfortunately, ets_vprintf in ROM
 * has been inlined by the compiler into ets_printf, so it is not accessible outside.)
 *
 * To override default verbosity level at file or component scope, define LOG_LOCAL_LEVEL macro.
 * At file scope, define it before including esp_log.h, e.g.:
 *
 *      #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
 *      #include "esp_log.h"
 *
 * At component scope, define it in component makefile:
 *
 *      CFLAGS += -D LOG_LOCAL_LEVEL=ESP_LOG_DEBUG
 *
 * To configure logging output per module at runtime, add calls to esp_log_set_level function:
 *
 *      esp_log_set_level("*", ESP_LOG_ERROR);        // set all components to ERROR level
 *      esp_log_set_level("wifi", ESP_LOG_WARN);      // enable WARN logs from WiFi stack
 *      esp_log_set_level("dhcpc", ESP_LOG_INFO);     // enable INFO logs from DHCP client
 *
 */


typedef enum {
    ESP_LOG_NONE,       // No log output
    ESP_LOG_ERROR,      // Critical errors, software module can not recover on its own
    ESP_LOG_WARN,       // Error conditions from which recovery measures have been taken
    ESP_LOG_INFO,       // Information messages which describe normal flow of events
    ESP_LOG_DEBUG,      // Extra information which is not necessary for normal use (values, pointers, sizes, etc).
    ESP_LOG_VERBOSE     // Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
} esp_log_level_t;

typedef int (*vprintf_like_t)(const char *, va_list);

/**
 * @brief Set log level for given tag
 *
 * If logging for given component has already been enabled, changes previous setting.
 *
 * @param tag Tag of the log entries to enable. Must be a non-NULL zero terminated string.
 *            Value "*" resets log level for all tags to the given value.
 *
 * @param level  Selects log level to enable. Only logs at this and lower levels will be shown.
 */
void esp_log_level_set(const char* tag, esp_log_level_t level);

/**
 * @brief Set function used to output log entries
 *
 * By default, log output goes to UART0. This function can be used to redirect log
 * output to some other destination, such as file or network.
 *
 * @param func Function used for output. Must have same signature as vprintf.
 */
void esp_log_set_vprintf(vprintf_like_t func);

/**
 * @brief Write message into the log
 *
 * This function is not intended to be used directly. Instead, use one of
 * ESP_LOGE, ESP_LOGW, ESP_LOGI, ESP_LOGD, ESP_LOGV macros.
 *
 * This function or these macros should not be used from an interrupt.
 */
void esp_log_write(esp_log_level_t level, const char* tag, const char* format, ...) __attribute__ ((format (printf, 3, 4)));


/**
 * @brief Function which returns timestamp to be used in log output
 *
 * This function is used in expansion of ESP_LOGx macros.
 * In the 2nd stage bootloader, and at early application startup stage
 * this function uses CPU cycle counter as time source. Later when
 * FreeRTOS scheduler start running, it switches to FreeRTOS tick count.
 *
 * For now, we ignore millisecond counter overflow.
 *
 * @return timestamp, in milliseconds
 */
uint32_t esp_log_timestamp();


#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V
#else //CONFIG_LOG_COLORS
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

#ifndef LOG_LOCAL_LEVEL
#ifndef BOOTLOADER_BUILD
#define LOG_LOCAL_LEVEL  ((esp_log_level_t) CONFIG_LOG_DEFAULT_LEVEL)
#else
#define LOG_LOCAL_LEVEL  ((esp_log_level_t) CONFIG_LOG_BOOTLOADER_LEVEL)
#endif
#endif

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%d) %s: " format LOG_RESET_COLOR "\n"

#define ESP_EARLY_LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR) { ets_printf(LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#ifndef BOOTLOADER_BUILD
#define ESP_LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR) { esp_log_write(ESP_LOG_ERROR, tag, LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#else
#define ESP_LOGE( tag, format, ... )  ESP_EARLY_LOGE( tag, format, ##__VA_ARGS__)
#endif  // BOOTLOADER_BUILD

#define ESP_EARLY_LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN) { ets_printf(LOG_FORMAT(W, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#ifndef BOOTLOADER_BUILD
#define ESP_LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN) { esp_log_write(ESP_LOG_WARN, tag, LOG_FORMAT(W, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#else
#define ESP_LOGW( tag, format, ... )  ESP_EARLY_LOGW( tag, format, ##__VA_ARGS__)
#endif  // BOOTLOADER_BUILD

#define ESP_EARLY_LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO) { ets_printf(LOG_FORMAT(I, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#ifndef BOOTLOADER_BUILD
#define ESP_LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO) { esp_log_write(ESP_LOG_INFO, tag, LOG_FORMAT(I, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#else
#define ESP_LOGI( tag, format, ... )  ESP_EARLY_LOGI( tag, format, ##__VA_ARGS__)
#endif  //BOOTLOADER_BUILD

#define ESP_EARLY_LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) { ets_printf(LOG_FORMAT(D, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#ifndef BOOTLOADER_BUILD
#define ESP_LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG) { esp_log_write(ESP_LOG_DEBUG, tag, LOG_FORMAT(D, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#else
#define ESP_LOGD( tag, format, ... )  ESP_EARLY_LOGD(tag, format, ##__VA_ARGS__)
#endif  // BOOTLOADER_BUILD

#define ESP_EARLY_LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { ets_printf(LOG_FORMAT(V, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#ifndef BOOTLOADER_BUILD
#define ESP_LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { esp_log_write(ESP_LOG_VERBOSE, tag, LOG_FORMAT(V, format), esp_log_timestamp(), tag, ##__VA_ARGS__); }
#else
#define ESP_LOGV( tag, format, ... )  ESP_EARLY_LOGV(tag, format, ##__VA_ARGS__)
#endif  // BOOTLOADER_BUILD

#ifdef __cplusplus
}
#endif


#endif /* __ESP_LOG_H__ */

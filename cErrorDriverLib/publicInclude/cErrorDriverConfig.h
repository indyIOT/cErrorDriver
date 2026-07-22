/** ***********************************************
 * @file cErrorDriverConfigs.h
 * @brief Private interface for the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "commonMacros.h"
#ifndef C_ERROR_DRIVER_CONFIG_H
#define C_ERROR_DRIVER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CUSTOM_ERROR_DRIVER_CONFIG

#define MAX_MODULE_COUNT                                                     100
#define MAX_ERROR_MESSAGE_LENGTH_BYTES                                       100
#define MAX_FILENAME_LENGTH_BYTES                                             52
#define END_OF_COMMON_ERRORS                                              0x0020
#define ERROR_MESSAGE_FULL                                              DEF_TRUE
#endif // CUSTOM_ERROR_DRIVER_CONFIG
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_ERROR_DRIVER_CONFIG_H */
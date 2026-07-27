/** ***********************************************
 * @file cErrorDriver.h
 * @brief Private interface for the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cErrorDriverConfig.h"
#include "cErrorDriverPub.h"

#ifndef C_ERROR_DRIVER_H
#define C_ERROR_DRIVER_H
#ifdef __cplusplus
extern "C" {
#endif
#pragma pack( push, 1 )
/**
 * @brief Structure for storing error information.
 * @note This structure is designed to be compact and 32 bit aligned.
 */
typedef struct 
{
    sErrorCompact_t _compact; /* Compact error information */
#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
    uint8_t _errorMessage[ MAX_ERROR_MESSAGE_LENGTH_BYTES ]; /* A message describing the error */
    uint8_t _filename[ MAX_FILENAME_LENGTH_BYTES ]; /* The filename where the error occurred */
#endif // ERROR_MESSAGE_ENABLED
} sErrorInfo_t;

#pragma pack( pop )


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_ERROR_DRIVER_H */

/** ***********************************************
 * @file cCommonErrorCodes.h
 * @brief Private interface for the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "CommonTypes.h"
#include "cErrorDriverPub.h"
#ifndef C_COMMON_ERROR_CODES_H
#define C_COMMON_ERROR_CODES_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef ERROR_NONE
#undef ERROR_NONE
#endif

/** Enumeration of Common Error Codes These are always going to be the first error 
 * codes of every Errorcode Enumeration */
typedef enum 
{
    ERROR_NONE                = 0x0000, /* No error */
    ERROR_INVALID_PARAMETER   = 0x0001, /* Invalid parameter */
    ERROR_OUT_OF_MEMORY       = 0x0002, /* Out of memory */
    ERROR_BUFFER_OVERFLOW     = 0x0003, /* Buffer overflow */
    ERROR_BUFFER_UNDERFLOW    = 0x0004, /* Buffer underflow */
    ERROR_NULL_POINTER        = 0x0005, /* Null pointer */
    ERROR_INVALID_STATE       = 0x0006, /* Invalid state */
    ERROR_TIMEOUT             = 0x0007, /* Timeout */
    ERROR_NOT_IMPLEMENTED     = 0x0008, /* Not implemented */
    ERROR_UNKNOWN             = 0x0009, /* Unknown error */
    ERROR_UNINITIALIZED       = 0x000A, /* Uninitialized */
    ERROR_ALREADY_INITIALIZED = 0x000B, /* Object previously been initialized.*/ 
    LAST_COMMON_ERROR_CODE,
} eCommonErrorCodes_t;

#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
#define GET_COMMON_ERROR_MESSAGE( errorCode, errorMessage ) \
    getCommonErrorMessageFromErrorCode( errorCode, errorMessage )
/**
 * @brief Function to get the error message corresponding to a common error code.
 * @param errorCode The common error code to get the message for.
 * @param errorMessage Pointer to a buffer to store the error message.
 * @returns A pointer to the error message string.
 */
extern sErrorInfo_t getCommonErrorMessageFromErrorCode( uint16_t const errorCode,
                                                        uint8_t const * errorMessage );
#else 
#define GET_COMMON_ERROR_MESSAGE( errorCode, errorMessage ) \
    BLANK_ERROR_STRUCT
#endif // ERROR_MESSAGE_FULL
                                                        
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_COMMON_ERROR_CODES_H */
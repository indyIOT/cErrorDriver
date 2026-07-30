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
#include "commonTypes.h"
#include "cErrorDriverPub.h"
#ifndef C_COMMON_ERROR_CODES_H
#define C_COMMON_ERROR_CODES_H
#ifdef __cplusplus
extern "C" {
#endif


#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
#define GET_COMMON_ERROR_MESSAGE( errorCode, errorMessage ) \
    getCommonErrorMessageFromErrorCode( errorCode, errorMessage )
/**
 * @brief Function to get the error message corresponding to a common error code.
 * @param errorCode The common error code to get the message for.
 * @param errorMessage Pointer to a buffer to store the error message.
 * @param autoStoreError Whether or not to store This error in the error driver.
 * @returns A pointer to the error message string.
 */
extern sErrorCompact_t getCommonErrorMessageFromErrorCode( uint16_t const errorCode,
                                                           bool const autoStoreError,
                                                        uint8_t const * errorMessage );
#else 
#define GET_COMMON_ERROR_MESSAGE( errorCode, errorMessage ) \
    BLANK_ERROR_STRUCT
#endif // ERROR_MESSAGE_FULL
                                                        
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_COMMON_ERROR_CODES_H */
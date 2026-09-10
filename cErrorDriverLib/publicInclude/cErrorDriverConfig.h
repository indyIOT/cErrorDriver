/** ***********************************************
 * @file cErrorDriverConfig.h
 * @brief Overridable configuration of the error driver.
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

    #ifndef END_OF_COMMON_ERRORS
        #define END_OF_COMMON_ERRORS                                      0x0020
    #endif
    #define WRITE_ERROR_TO_MEMORY                                       DEF_TRUE
    #define MAX_FILENAME_LENGTH_BYTES                                         52
    #ifdef ERROR_MESSAGE_FULL
        #undef ERROR_MESSAGE_FULL
    #endif
    #define ERROR_MESSAGE_FULL                                          DEF_TRUE

    /* If we have a full error message then no matter what we are logging the 
       whole message. */
    #if( ERROR_MESSAGE_FULL == DEF_TRUE )
        #ifdef LOG_FULL_ERROR_MESSAGE
            #undef LOG_FULL_ERROR_MESSAGE
        #endif
        #define LOG_FULL_ERROR_MESSAGE                                  DEF_TRUE
    #else
        #ifndef LOG_FULL_ERROR_MESSAGE
            #define LOG_FULL_ERROR_MESSAGE                              DEF_TRUE
        #endif
    #endif // LOG_FULL_ERROR_MESSAGE

    #ifndef MAX_ERROR_MESSAGE_LENGTH_BYTES
        #if( ( ERROR_MESSAGE_FULL == DEF_TRUE ) || ( LOG_FULL_ERROR_MESSAGE == DEF_TRUE ) )
            #define MAX_ERROR_MESSAGE_LENGTH_BYTES                               100
        #else
            #define MAX_ERROR_MESSAGE_LENGTH_BYTES                                 1
        #endif  
    #endif // MAX_ERROR_MESSAGE_LENGTH_BYTES

    #ifndef VERIFY_MEMORY_WRITE
        #define VERIFY_MEMORY_WRITE                                       DEF_TRUE
    #endif // Verify Memory Write


#endif // CUSTOM_ERROR_DRIVER_CONFIG

    
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_ERROR_DRIVER_CONFIG_H */
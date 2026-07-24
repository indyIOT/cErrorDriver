/** ***********************************************
 * @file cCommonErrors.c
 * @brief Implementation of common errors for the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "CommonTypes.h"
#include "cErrorDriverPub.h"
#include "cCommonErrorCodes.h"

/** Error messsages only compiled and used if full errror message is enabled.*/
#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
/** Typedefs used for the driver  */
typedef struct
{
    sCommonDriverControlStruct_t _driverControl; /* The error code for this error */
} sCommonErrorControlStruct_t;
/************************** Static Global Variables ********************************/
/**
 * @brief All source files should have a module name and ID. This allows for more precise error reporting.
 */
static const uint8_t moduleName[] = "CommonErrors";

/**
 * @brief Control structure for this file allowing static variables to be called from a THIS pointer.
 */
static const sCommonErrorControlStruct_t commonErrorModuleControl = 
{
    ._driverControl = {
        ._driverInfo = {
            ._moduleName = moduleName,
            ._moduleID   = 0x0000,
            ._isInitialized = true
        },
        ._driverAccessors = {
            .getModuleIdFunction = NULL,
            .getModuleVersionStringFunction = NULL,
            .getModuleNameFunction = NULL,
            .isDriverInitializedFunction = NULL
        }
    }
};

/**
 * @brief Pointer to the control structure.
 */
static sCommonErrorControlStruct_t const * const THIS = &commonErrorModuleControl;

/**
 * @brief Array of Error codes and their corresponding error messages. This is used to retrieve the error message given an error code.
 *        The error codes are defined in cCommonErrorCodes.h and the error messages are defined in this file. 
 *        The error codes are offset by the module ID to ensure that they are unique across all modules.
 */
static const sErrorCodeMessagePair_t commonErrorCodeMessagePairs[] = 
{
    { ERROR_NONE,               "" },
    { ERROR_INVALID_PARAMETER,  "Invalid parameter" },
    { ERROR_OUT_OF_MEMORY,      "Out of memory" },
    { ERROR_BUFFER_OVERFLOW,    "Buffer overflow" },
    { ERROR_BUFFER_UNDERFLOW,   "Buffer underflow" },
    { ERROR_NULL_POINTER,       "Null pointer" },
    { ERROR_INVALID_STATE,      "Invalid state" },
    { ERROR_TIMEOUT,            "Timeout" },
    { ERROR_NOT_IMPLEMENTED,    "Not implemented" },
    { ERROR_UNKNOWN,            "Unknown error" },
    { ERROR_UNINITIALIZED,      "Uninitialized" },
    { ERROR_ALREADY_INITIALIZED,"Already initialized" },
    { LAST_COMMON_ERROR_CODE,   "LAST" }
};

/************************************************** Extern Functions **************************************/
/**
 * @brief Function to get the error message corresponding to a common error code.
 * @param errorCode The common error code to get the message for.
 * @param errorMessage Pointer to a buffer to store the error message.
 * @returns An error struct with ERROR_NONE if successful, or an error struct with an error code if unsuccessful.
 */
sErrorInfo_t getCommonErrorMessageFromErrorCode( uint16_t const errorCode,
                                                 uint8_t const * errorMessage )
{
    sErrorInfo_t retValue = BLANK_ERROR_STRUCT;
    errorMessage = NULL;
    /**
     * range check the code.
     */
    if( errorCode < LAST_COMMON_ERROR_CODE )
    {
        retValue = CREATE_ERROR( ERROR_INVALID_PARAMETER, 
                                 commonErrorCodeMessagePairs[ERROR_INVALID_PARAMETER]._errorMessage );
    }
    else
    {
        errorMessage = commonErrorCodeMessagePairs[errorCode]._errorMessage;
    }
   
    return ( retValue );
}

/****************************************************Static Functions  ***********************************/
#endif // ERROR_MESSAGE_FULL

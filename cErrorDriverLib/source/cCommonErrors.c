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
#include "cErrorDriverConfig.h"
#include "cErrorDriverPub.h"
#include "cCommonErrorCodes.h"

#ifdef __cplusplus
extern "C" {
#endif
/** Error messsages only compiled and used if full errror message is enabled.*/
#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
/******************************** Type definitions ****************************/

typedef struct
{
    sCommonDriverControlStruct_t _driverControl; /* The error code for this error */
} sCommonErrorControlStruct_t;

/********************************Static functions Prototypes *************/
static uint16_t getModuleId( void );
static uint8_t const * getModuleVersionString( void );
static sCommonVersionStruct_t getModuleVersion( void );
static uint8_t const * getModuleName( void );
static bool isDriverInitialized( void );

/******************************** Static Global Variables **********************/
static const uint8_t moduleName[] = "cCommonErrors";
#define MODULE_ID 54556



/**
 * @brief Control structure for this file allowing static variables to be called from a THIS pointer.
 */
static const sCommonErrorControlStruct_t commonErrorModuleControl = 
{
    ._driverControl = {
        ._driverInfo = {
            ._moduleName = moduleName,
            ._moduleID   = MODULE_ID,
            ._isInitialized = true
        },
        ._driverAccessors = {
            .getModuleIdFunction = getModuleId,
            .getModuleVersionStringFunction = getModuleVersionString,
            .getModuleNameFunction = getModuleName,
            .isDriverInitializedFunction = isDriverInitialized
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

/**************************** HELPER MACROS ************************************/
#ifndef ERROR_NONE
#define ERROR_NONE 0U
#endif

#ifndef NO_ERROR
#define NO_ERROR 0U
#endif

/****************************** Function implementations ***************/
/**
 * @brief Function to get the error message corresponding to a common error code.
 * @param errorCode The common error code to get the message for.
 * @param errorMessage Pointer to a buffer to store the error message.
 * @returns An error struct with ERROR_NONE if successful, or an error struct with an error code if unsuccessful.
 */
sErrorCompact_t getCommonErrorMessageFromErrorCode( uint16_t const errorCode,
                                                    bool const autoStoreError,
                                                    uint8_t const * errorMessage )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    errorMessage = commonErrorCodeMessagePairs[ERROR_INVALID_PARAMETER]._errorMessage;;
    /**
     * range check the code.
     */
    if( errorCode < LAST_COMMON_ERROR_CODE )
    {        
        retValue = CREATE_STORE_ERROR( ERROR_INVALID_PARAMETER,  
                                       autoStoreError,                               
                                       commonErrorCodeMessagePairs[ERROR_INVALID_PARAMETER]._errorMessage );
    }
    else
    {
        errorMessage = commonErrorCodeMessagePairs[errorCode]._errorMessage;
    }
   
    return ( retValue );
}

/************************ Static Function Implementations ***************/
/**
 * @brief Function to get the module ID of the error driver.
 */
static uint16_t getModuleId( void )
{
    return ( THIS->_driverControl._driverInfo._moduleID );
}

/**
 * @brief Function to get the module version string of the error driver.
 */
static uint8_t const * getModuleVersionString( void )
{
    return ( THIS->_driverControl._driverInfo._moduleVersionString );
}

/**
 * @brief Function to get the module version of the error driver.
 */
static sCommonVersionStruct_t getModuleVersion( void )
{
    return ( THIS->_driverControl._driverInfo._moduleVersion );
}

/**
 * @brief Function to get the module name of the error driver.
 */
static uint8_t const * getModuleName( void )
{
    return ( THIS->_driverControl._driverInfo._moduleName );
}

/**
 * @brief Function to check if the error driver is initialized.
 */
static bool isDriverInitialized( void )
{
    return ( THIS->_driverControl._driverInfo._isInitialized );
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // ERROR_MESSAGE_FULL










/************************************************** Extern Functions **************************************/


/****************************************************Static Functions  ***********************************/


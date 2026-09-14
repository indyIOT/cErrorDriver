/** ***********************************************
 * @file cErrorDriver.c
 * @brief Implementation of the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "commonMacros.h"
#include "commonTypes.h"
#include "cErrorDriver.h"
#include "cErrorDriverPub.h"
#include "cErrorDriverVersion.h"
#include "cCommonErrorCodes.h"
#ifdef __cplusplus
extern "C" {
#endif

/******************************** Type definitions ****************************/
typedef struct
{
    sCommonDriverControlStruct_t _driverControl; /* Control structure for the error driver */
    logCallback_t logMessageFunction; /* Pointer to a function for logging errors */
    readMemoryFunctionPtr_t readMemoryFunction; /* Pointer to a function that reads memory for the error driver */
    writeMemoryFunctionPtr_t writeMemoryFunction; /* Pointer to a function for writing memory for the error driver */
    calculateCRC16FunctionPtr_t CRC16Function; /* Pointer to a function for calculating CRC16 */
    uint32_t _memoryAddress; /* The starting address of the memory to be used by the error driver */
    uint16_t _memorySizeInBytes; /* Size of the memory in bytes */
} sErrorDriverControlStruct_t;


/********************************Static functions Prototypes *************/
static uint16_t getModuleId( void );
static uint8_t const * getModuleVersionString( void );
static sCommonVersionStruct_t getModuleVersion( void );
static uint8_t const * getModuleName( void );
static bool isDriverInitialized( void );

/******************************** Static Global Variables **********************/
static const uint8_t moduleName[] = "cErrorDriver";
#define MODULE_ID 31905

#if ( ( ERROR_MESSAGE_FULL == DEF_TRUE ) || ( LOG_FULL_ERROR_MESSAGE == DEF_TRUE ) )

    /** @brief A string to use when no error message is available. */
    static const uint8_t noErrorMessage[] = "No error message available.";

    /** @brief No module name available. */
    static const uint8_t noModuleName[] = "NoModule";
    /**
     * @brief Array of Error codes and their corresponding error messages. 
     *        This is used to retrieve the error message given an error code.
     *        The error codes are defined in cCommonErrorCodes.h and the error messages 
     *        are defined in this file. 
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
#endif

static sErrorDriverControlStruct_t errorDriverControlStruct = { 
    ._driverControl = { 
        ._driverInfo = {
                        ._moduleName = moduleName,
                        ._moduleVersionString = ERROR_DRIVER_VERSION_STRING,
                        ._moduleID = MODULE_ID,                        
                        ._moduleVersion = { ._major = ERROR_DRIVER_VERSION_MAJOR,
                                            ._minor = ERROR_DRIVER_VERSION_MINOR,
                                            ._patch = ERROR_DRIVER_VERSION_PATCH,
                                            ._buildType = ERROR_DRIVER_VERSION_BUILD_TYPE_ENUM
                        },
                         ._isInitialized = false
},
        ._driverAccessors = { 
                            .getModuleIdFunction = getModuleId,
                            .getModuleVersionStringFunction = getModuleVersionString,
                            .getModuleNameFunction = getModuleName,
                            .getModuleVersionFunction = getModuleVersion,
                            .isDriverInitializedFunction = isDriverInitialized },
    },
    .logMessageFunction = NULL,
    .readMemoryFunction = NULL, 
    .writeMemoryFunction = NULL, 
    ._memoryAddress = 0U, 
    ._memorySizeInBytes = 0U 
};

static sErrorDriverControlStruct_t * const THIS = &errorDriverControlStruct;

/**************************** HELPER MACROS ************************************/
#ifndef ERROR_NONE
#define ERROR_NONE 0U
#endif

#ifndef NO_ERROR
#define NO_ERROR 0U
#endif


/****************************** Function implementations ***************/
/**
 * @brief Enter a spin loop after a debug assert failure.
 * @note A debugger can change keepSpinning to false to escape the loop.
 */
void cErrorDriverDebugAssertSpin( char const * const expression,
                                  char const * const fileName,
                                  unsigned int lineNumber )
{
    static volatile bool keepSpinning = true;

    AG_UNUSED( expression );
    AG_UNUSED( fileName );
    AG_UNUSED( lineNumber );

    while( keepSpinning )
    {
        AG_NOP();
    }
}

/**
 * @brief Function to initialize the error driver. This should be called before any other functions are used.
 * @param logCallback Pointer to a function for logging errors. This is used to log errors to the logging driver.
 * @param readMemory Pointer to a function that reads memory for the error driver. 
 *                   This is used to read the error information from the circular buffer.
 * @param writeMemory Pointer to a function for writing memory for the error driver.
 * @param memoryAddress The starting address of the memory to be used by the error driver.
 * @param memorySizeInBytes Size of the memory in bytes
 * @return an error structure if unsuccessful, or ERROR_NONE if successful.
 */
sErrorCompact_t initErrorDriver( readMemoryFunctionPtr_t readMemory,
                                        writeMemoryFunctionPtr_t writeMemory,
                                        logCallback_t logCallback,
                                        calculateCRC16FunctionPtr_t calculateCRC16Function,
                                        uint32_t memoryAddress,
                                        uint16_t memorySizeInBytes )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    
    if( THIS->_driverControl._driverInfo._isInitialized == false )
    {
        /** Log callback can be null if logging is not required  */
        THIS->logMessageFunction = logCallback;
        THIS->_memoryAddress = memoryAddress;
        THIS->_memorySizeInBytes = memorySizeInBytes;
        /* These functions can be null esp if we are not writing to memory */
        THIS->readMemoryFunction = NULL;
        THIS->writeMemoryFunction = NULL;
        #if ( WRITE_ERROR_TO_MEMORY == DEF_TRUE )
        if( ( readMemory == NULL ) || 
             ( writeMemory == NULL ) )
        {
            retValue = CREATE_ERROR( ERROR_NULL_POINTER, NULL );
            if( THIS->logMessageFunction != NULL )
            {
                (void)THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, 
                                          __LINE__, 
                                          LOGGING_TYPE_CRITICAL, 
                                          "Error Driver Initialization Failed: Read or Write Memory function pointer is NULL." );
            }            
        }
        else if( calculateCRC16Function == NULL )
        {
            retValue = CREATE_ERROR( ERROR_NULL_POINTER, NULL );
            if( THIS->logMessageFunction != NULL )
            {
                (void)THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, 
                                          __LINE__, 
                                          LOGGING_TYPE_CRITICAL, 
                                          "Error Driver Initialization Failed: Calculate CRC16 function pointer is NULL." );
            }            
        }
        else if( memorySizeInBytes < sizeof( sErrorCompact_t ) )
        {
            // Memory size is too small to store even one error compact structure.
            retValue = CREATE_ERROR( ERROR_INVALID_PARAMETER, NULL );
            if( THIS->logMessageFunction != NULL )
            {
                (void)THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, 
                                        __LINE__, 
                                        LOGGING_TYPE_CRITICAL, 
                                        "Error Driver Initialization Failed: Memory size is too small. Minimum size required is %u bytes.", sizeof( sErrorCompact_t ) );
            }
        }     
        else
        {
            THIS->readMemoryFunction = readMemory;
            THIS->writeMemoryFunction = writeMemory;
            THIS->CRC16Function = calculateCRC16Function;
        }
        #endif
        /* If there were no errors while initializing the driver. */
        if ( retValue._errorCode == ERROR_NONE )        
        {
            /* Initialize the circular buffer here. */
            THIS->_driverControl._driverInfo._isInitialized = true;
        }
    }
    else
    {
        retValue = CREATE_ERROR( ERROR_ALREADY_INITIALIZED, NULL );
    }
    return ( retValue );
}

#ifdef UNIT_TESTS
/**
 * @brief Test-only hook that resets the error driver back to an uninitialized state.
 * @note Compiled only when UNIT_TESTS is defined. See cErrorDriverPub.h.
 */
void resetErrorDriverForTest( void )
{
    THIS->_driverControl._driverInfo._isInitialized = false;
    THIS->logMessageFunction = NULL;
    THIS->readMemoryFunction = NULL;
    THIS->writeMemoryFunction = NULL;
    THIS->CRC16Function = NULL;
    THIS->_memoryAddress = 0U;
    THIS->_memorySizeInBytes = 0U;
}
#endif

/**
 * @brief Function to create an ERROR structure given an error code, line number, filename,
 *        and an error message.
 * @param errorMessage A message describing the error
 * @param fileModuleEnum The module ID where the error occurred.
 * @param moduleName The name of the module where the error occurred.
 * @param lineNumber The line number where the error occurred
 */
sErrorCompact_t createErrorCompact( uint16_t errorCode, 
                                 uint16_t fileModuleEnum, 
                                 uint16_t lineNumber,                                 
                                 bool autoStoreError,
                                 uint8_t const * const errorMessage, 
                                 uint8_t const * const moduleName )
{    
    sErrorInfo_t errorInfo = { 0 };
    sErrorCompact_t * retValue = &errorInfo._compact;
    uint16_t writeCheck = ERROR_NONE;
    size_t errorMessageLength = 0;
    uint8_t const * errorMessagePtr = errorMessage;
    uint8_t const * filenamePtr = moduleName;
    sCRC16Config_t crc16Config = DEFAULT_CRC16_CONFIG;
    /* Only fill this structure out if the error code is non-zero */
    if( errorCode != ERROR_NONE )
    {
        retValue->_errorCode = errorCode;
        retValue->_fileModuleEnum = fileModuleEnum;
        retValue->_lineNumber = lineNumber;
        if( filenamePtr == NULL )
        {
            filenamePtr = noModuleName;
        }

        /* Only continue on if the driver is initialized */
        if( THIS->_driverControl._driverInfo._isInitialized == true ) 
        {
#if ( ( ERROR_MESSAGE_FULL == DEF_TRUE ) || ( LOG_FULL_ERROR_MESSAGE == DEF_TRUE ) )

            /** If the error message is null or empty then we should use the no error message available. */
            if( ( errorMessagePtr == NULL ) || 
                ( strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 ) == 0 ) )
            {
                errorMessagePtr = noErrorMessage;
            }

            /** If this is a common error then get the appropriate message. */
            if( ( errorCode < END_OF_COMMON_ERRORS ) && 
                ( ( errorMessagePtr == NULL ) || 
                  ( strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 ) == 0 ) ) )
            {
                /* Error discarded here because an error is handled gracefully within
                the called function.  */
                ( void )getErrorMessageFromErrorCode( commonErrorCodeMessagePairs, 
                                                      LAST_COMMON_ERROR_CODE, 
                                                      errorCode, 
                                                      false, 
                                                      errorMessagePtr );
            }

        
            /* If we still have no error message then we should use the no error message available. */
            if( ( errorMessagePtr == NULL ) || 
                ( strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 ) == 0 ) )
            {
                errorMessagePtr = noErrorMessage;
            }
#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
            /* CHeck to ensure that the message exists before logging it to the
             error log or the system log. */
            if( errorMessagePtr != NULL )
            {
                /** If the error log is storing the entire error message. */     
                if( ( autoStoreError == true ) && ( THIS->writeMemoryFunction != NULL ) )
                {
                    errorMessageLength = strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 );
                    (void)memcpy( (void*)&errorInfo._errorMessage[0], (void*)errorMessagePtr, errorMessageLength );
                    errorInfo._errorMessage[errorMessageLength] = '\0';
                }

            }

            /** 
             * Place the module name into the error info structure. If the module name is null then we should use the no module name available.
             * If the module name is not null then we should use the provided module name.    
             */
            if( filenamePtr != NULL )
            {
                (void)memcpy( (void*)&errorInfo._filename[0], (void*)filenamePtr, strnlen( (const char *)filenamePtr, MAX_FILENAME_LENGTH_BYTES - 1 ) );
            }
            else
            {
                (void)memcpy( (void*)&errorInfo._filename[0], (void*)noModuleName, strnlen( (const char *)noModuleName, MAX_FILENAME_LENGTH_BYTES - 1 ) );
            }
#endif

#endif
            /* Only call the CRC if the calcualte crc is not null */
            if( THIS->CRC16Function != NULL )
            {
                /* Add the two CRCs onto the error info and compact structures. */
                (void)THIS->CRC16Function( &crc16Config, (uint8_t const * const)retValue, sizeof( sErrorCompact_t ) - sizeof( retValue->_crc16 ), &retValue->_crc16 );
                (void)THIS->CRC16Function( &crc16Config, (uint8_t const * const)&errorInfo, sizeof( sErrorInfo_t ) - sizeof( errorInfo._crc16 ), &errorInfo._crc16 );
                /* If the CRC error failed we are already in an error state and cascading errors are not handled other than by console log. */
            }

#if ( LOG_FULL_ERROR_MESSAGE == DEF_TRUE )
            /* Log the message to teh system log if the logmessage function is set. */
            if( THIS->logMessageFunction != NULL )
            {
                if( errorMessagePtr != NULL )
                {
                    // Log Error without a message.
                    (void)THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, 
                                            __LINE__, 
                                            LOGGING_TYPE_ERROR, 
                                            "Error Code: 0x%04X, Module: %s, Line: %u, Message: %s", errorCode, moduleName, lineNumber, errorMessagePtr );
                }
                else
                {
                    // Log error without a message. 
                    (void)THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, 
                                            __LINE__, 
                                            LOGGING_TYPE_ERROR, 
                                            "Error Code: 0x%04X, Module: %s, Line: %u, Message: No error message available.", errorCode, moduleName, lineNumber );
                }
            }
#endif
            
            /* If the driver is initialized then add the error info to the circular buffer.
             If it is not initialized then we cannot log the error to the circular buffer. */
            if( ( autoStoreError == true ) && 
                ( THIS->writeMemoryFunction != NULL ) )
            {
                writeCheck = THIS->writeMemoryFunction( THIS->_memoryAddress, 
                                                                 (uint8_t*)&errorInfo, 
                                                                 sizeof( sErrorInfo_t ), 
                                                                 VERIFY_MEMORY_WRITE );
                if( writeCheck != ERROR_NONE )
                {
                    if( THIS->logMessageFunction != NULL )
                    {
                        (void)THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, 
                                                  __LINE__, 
                                                  LOGGING_TYPE_CRITICAL, 
                                                  "Error Driver: Failed to write error info to memory. Error Code: 0x%04X", writeCheck );
                    }
                }
            }
        }
    }

    return ( *retValue );
}

/**
 * @brief Function to get the Error driver information.
 *        This will return a structure containing accessors to
 *        get the module ID, version string, and other information
 *        about the error driver.
 */
sCommonDriverAccessorStruct_t const * const getErrorDriverInfoAccessors( void )
{
    return (sCommonDriverAccessorStruct_t const * const)( &THIS->_driverControl._driverAccessors );
}

#if ( ( ERROR_MESSAGE_FULL == DEF_TRUE ) || ( LOG_FULL_ERROR_MESSAGE == DEF_TRUE ) )
/**
 * @brief Function to get the error message corresponding error error code.
 * @param errorCode The common error code to get the message for.
 * @param errorMessage Pointer to a buffer to store the error message.
 * @returns An error struct with ERROR_NONE if successful, or an error struct with an error code if unsuccessful.
 */
sErrorCompact_t getErrorMessageFromErrorCode( sErrorCodeMessagePair_t const * const errorCodeMessagePairs,
                                              uint16_t const lastErrorCode,
                                              uint16_t const errorCode,
                                              bool const autoStoreError,
                                              uint8_t const * errorMessage )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    errorMessage = noErrorMessage;
    /**
     * range check the code.
     */
    if( errorCode >= lastErrorCode )
    {        
        retValue = CREATE_STORE_ERROR( ERROR_INVALID_PARAMETER,  
                                       autoStoreError,                               
                                       commonErrorCodeMessagePairs[ERROR_INVALID_PARAMETER]._errorMessage );
    }
    else
    {
        errorMessage = errorCodeMessagePairs[errorCode]._errorMessage;
    }
   
    return ( retValue );
}
#endif
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

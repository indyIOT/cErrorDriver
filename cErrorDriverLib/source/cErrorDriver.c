/** ***********************************************
 * @file cErrorDriver.c
 * @brief Implementation of the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#ifdef __cplusplus
extern "C" {
#endif

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
/********************************Module Type definitions **********************/
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


/********************************Static Module Accessor functions Prototypes *************/
static uint16_t getModuleId( void );
static uint8_t const * getModuleVersionString( void );
static sCommonVersionStruct_t getModuleVersion( void );
static uint8_t const * getModuleName( void );
static bool isDriverInitialized( void );

/********************************Static Global Variables **********************/
static const uint8_t moduleName[] = "ErrorDriver";
static const uint8_t noErrorMessage[] = "No error message available.";


static sErrorDriverControlStruct_t errorDriverControlStruct = { 
    ._driverControl = { 
        ._driverInfo = { ._isInitialized = false,
                        ._moduleID = 0,
                        ._moduleVersionString = ERROR_DRIVER_VERSION_STRING,
                        ._moduleVersion = { ._major = ERROR_DRIVER_VERSION_MAJOR,
                                            ._minor = ERROR_DRIVER_VERSION_MINOR,
                                            ._patch = ERROR_DRIVER_VERSION_PATCH,
                                            ._buildType = ERROR_DRIVER_VERSION_BUILD_TYPE_ENUM
                        },
                        ._moduleName = moduleName },
        ._driverAccessors = { .getModuleIdFunction = getModuleId,
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

/**************************** HELPER MACROS ***********************************************/
#define LOG_CRIT( message, ... ) \
    ( THIS->logMessageFunction != NULL ) ? \
        THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, \
                                  __LINE__, \
                                  LOGGING_TYPE_CRITICAL, \
                                  message, ##__VA_ARGS__ ) \
    : BLANK_ERROR_STRUCT 


#define LOG_ERR( message, ... )  \
    ( THIS->logMessageFunction != NULL ) ? \
        THIS->logMessageFunction( THIS->_driverControl._driverInfo._moduleID, \
                                  __LINE__, \
                                  LOGGING_TYPE_ERROR, \
                                  message, ##__VA_ARGS__ ) \
    : BLANK_ERROR_STRUCT 
/****************************** Module Function implementations ***************/

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
        if( readMemory == NULL || writeMemory == NULL )
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
        else 
        {
            if( memorySizeInBytes < sizeof( sErrorCompact_t ) )
            {
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
        }
        #endif
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
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    sErrorInfo_t errorInfo = { 0 };
    uint16_t writeCheck = ERROR_NONE;
    size_t errorMessageLength = 0;
    uint8_t const * errorMessagePtr = errorMessage;
    sCRC16Config_t crc16Config = DEFAULT_CRC16_CONFIG;
    /* Only fill this structure out if the error code is non-zero */
    if( errorCode != ERROR_NONE )
    {
        retValue._errorCode = errorCode;
        retValue._fileModuleEnum = fileModuleEnum;
        retValue._lineNumber = lineNumber;
        errorInfo._compact = retValue;
        /* Only continue on if the driver is initialized */
        if( THIS->_driverControl._driverInfo._isInitialized == true ) 
        {
            /** If this is a common error then get the appropriate message. */
            if( ( errorCode < END_OF_COMMON_ERRORS ) && 
                ( ( errorMessagePtr == NULL ) || 
                  ( strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 ) == 0 ) ) )
            {
                ( void )getCommonErrorMessageFromErrorCode( errorCode, false, errorMessagePtr );
            }

        
            /* If we still have no error message then we should use the no error message available. */
            if( ( errorMessagePtr == NULL ) || 
                ( strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 ) == 0 ) )
            {
                errorMessagePtr = noErrorMessage;
            }

            /* CHeck to ensure that the message exists before logging it to the
             error log or the system log. */
            if( errorMessagePtr != NULL )
            {
                /** If the error log is storing the entire error message. */     
#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
                if( ( autoStoreError == true ) && ( THIS->writeMemoryFunction != NULL ) )
                {
                    errorMessageLength = strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 );
                    (void)memcpy( (void*)&errorInfo._errorMessage[0], (void*)errorMessagePtr, errorMessageLength );
                    errorInfo._errorMessage[errorMessageLength] = '\0';
                }
#endif
            }

            /* Only call the CRC if the calcualte crc is not null */
            if( THIS->CRC16Function != NULL )
            {
                /** Add the two CRCs onto the error info and compact structures. */
                retValue._crc16 = THIS->CRC16Function( crc16Config, (uint8_t const * const)&retValue, sizeof( sErrorCompact_t ) - sizeof( retValue._crc16 ) );
                errorInfo._crc16 = THIS->CRC16Function( crc16Config, (uint8_t const * const)&errorInfo, sizeof( sErrorInfo_t ) - sizeof( errorInfo._crc16 ) );
            }


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

    return ( retValue );
}

/***************************************Static Function Implementations ******/
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

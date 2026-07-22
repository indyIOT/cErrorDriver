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
#include "cLoggingDriverPub.h"
#include "cErrorDriverVersion.h"
#include "cCommonErrorCodes.h"

 
static const uint8_t moduleName[] = "ErrorDriver";
static uint8_t const * const getModuleName( void );

typedef struct
{
    sCommonDriverControlStruct_t _driverControl; /* Control structure for the error driver */
    readMemoryFunctionPtr_t readMemoryFunction; /* Pointer to a function that reads memory for the error driver */
    writeMemoryFunctionPtr_t writeMemoryFunction; /* Pointer to a function for writing memory for the error driver */
    uint32_t _memoryAddress; /* The starting address of the memory to be used by the error driver */
    uint16_t _memorySizeInBytes; /* Size of the memory in bytes */
} sErrorDriverControlStruct_t;


static sErrorDriverControlStruct_t errorDriverControlStruct = { 
    ._driverControl = { 
        ._driverInfo = { ._isInitialized = false,
                        ._moduleID = 0 ,
                        ._moduleName = moduleName },
        
    }, .readMemoryFunction = NULL, .writeMemoryFunction = NULL, ._memoryAddress = 0, ._memorySizeInBytes = 0 };
static sErrorDriverControlStruct_t * const THIS = &errorDriverControlStruct;

/** 
 * @brief Get the name of the error module.
 * @return Pointer to a string containing the module name.
 */
static uint8_t const * const getModuleName( void )
{
    return (uint8_t const * const)moduleName;
}

/**
 * @brief Get the version string of the error module.
 * @return Pointer to a string containing the version of the module.
 */
uint8_t const * getModuleVersionString( void )
{
    return (uint8_t const *)ERROR_DRIVER_VERSION_STRING;
}

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
 * @param readMemory Pointer to a function that reads memory for the error driver. 
 *                   This is used to read the error information from the circular buffer.
 * @param writeMemory Pointer to a function for writing memory for the error driver.
 * @param memoryAddress The starting address of the memory to be used by the error driver.
 * @param memorySizeInBytes Size of the memory in bytes
 * @return an error structure if unsuccessful, or ERROR_NONE if successful.
 */
sErrorInfo_t initErrorDriver( readMemoryFunctionPtr_t readMemory,
                              writeMemoryFunctionPtr_t writeMemory,
                              uint32_t memoryAddress,
                              uint16_t memorySizeInBytes )
{
    sErrorInfo_t retValue = BLANK_ERROR_STRUCT;
    if( THIS->_driverControl._driverInfo._isInitialized == false )
    {
        if( readMemory == NULL || writeMemory == NULL )
        {
            retValue = CREATE_ERROR( ERROR_NULL_POINTER, "" );
            LOG_CRITICAL( "Error Driver Initialization Failed: Read or Write Memory function pointer is NULL." );
        }
        else
        {
            if( memorySizeInBytes < sizeof( sErrorInfo_t ) )
            {
                retValue = CREATE_ERROR( ERROR_INVALID_PARAMETER, "" );
                LOG_CRITICAL( "Error Driver Initialization Failed: Memory size is too small. Minimum size required is %u bytes.", sizeof( sErrorInfo_t ) );
            }
            else
            {
                THIS->_driverControl._driverInfo.readMemoryFunction = readMemory;
                THIS->_driverControl._driverInfo.writeMemoryFunction = writeMemory;
                THIS->_driverControl._driverInfo._memoryAddress = memoryAddress;
                THIS->_driverControl._driverInfo._memorySizeInBytes = memorySizeInBytes;
                THIS->_driverControl._driverInfo._isInitialized = true;
            }
        }
    }
    else
    {
        retValue = CREATE_ERROR( ERROR_ALREADY_INITIALIZED, "" );
    }
    return retValue;
}

/**
 * @brief Function to create an ERROR structure given an error code, line number, filename, 
 *        and an error message.
 * @param errorMessage A message describing the error
 * @param fileModuleEnum The module ID where the error occurred.
 * @param moduleName The name of the module where the error occurred.
 * @param lineNumber The line number where the error occurred
 */
sErrorInfo_t createErrorInfo( uint16_t errorCode, 
                                 uint16_t fileModuleEnum, 
                                 uint16_t lineNumber, 
                                 uint8_t const * const errorMessage, 
                                 uint8_t const * const moduleName )
{
    sErrorInfo_t retValue = BLANK_ERROR_STRUCT;
    sErrorInfo_t functionError = BLANK_ERROR_STRUCT;
    size_t errorMessageLength = 0;
    uint8_t const * errorMessagePtr = errorMessage;

    /** Only fill this structure out if the error code is non-zero */
    if( errorCode != ERROR_NONE )
    {
        retValue._compact._errorCode = errorCode;
        retValue._compact._fileModuleEnum = fileModuleEnum;
        retValue._compact._lineNumber = lineNumber;
        /** Common Errors are just that. */
        if( errorCode < END_OF_COMMON_ERRORS )
        {
            functionError = getCommonErrorMessageFromErrorCode( errorCode, errorMessagePtr );
        }

        /* If there was no error retrieving the common error message or it was not a common
           error. */
        if( functionError._compact._errorCode == ERROR_NONE )
        {
            errorMessageLength = strnlen( (const char *)errorMessagePtr, MAX_ERROR_MESSAGE_LENGTH_BYTES - 1 );
            (void)memcpy( (void*)&retValue._errorMessage[0], (void*)errorMessagePtr, errorMessageLength );
            retValue._errorMessage[errorMessageLength] = '\0';           

            (void)LOG_ERROR( "Error Code: 0x%04X, Module: %s, Line: %u, Message: %s", errorCode, moduleName, lineNumber, retValue._errorMessage );
            /* If the driver is initialized then add the error info to the circular buffer.
             If it is not initialized then we cannot log the error to the circular buffer. */
            if( THIS->isInitialized == true )
            {
                (void)THIS->writeMemory( THIS->memoryAddress, (uint8_t*)&retValue, sizeof( sErrorInfo_t ) );
            }

        }
        else
        {
            retValue = functionError;
        }
    }

    return retValue;
}




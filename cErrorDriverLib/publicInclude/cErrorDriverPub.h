/** ***********************************************
 * @file cErrorDriverPub.h
 * @brief Public interface for the error driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cErrorDriverConfig.h"
#include "commonTypes.h"
#ifndef C_ERROR_DRIVER_PUB_H
#define C_ERROR_DRIVER_PUB_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************Public Typedefs ***********************************/
#ifndef SCOMMON_ERROR_COMPACT
#define SCOMMON_ERROR_COMPACT
/**
 * @brief Compact structure for storing error information.
 *  Needs to be 32 bit aligned and packed to 1 byte to ensure that it is compact and can be stored in a circular buffer.
 */
#pragma pack( push, 1 )
typedef struct 
{
    union {
        struct         
        {
            /* data */
            uint16_t _errorCode; /* The error code for this error */
            uint16_t _fileModuleEnum; /* The file module enum where the error occurred */
        };
        uint32_t _errorCodeAndFileModuleEnum; /* Combined error code and file module enum for compact storage */        
    };
    uint16_t _lineNumber; /* The line number where the error occurred */
    uint8_t  _flags; /* Whether or not this error info is valid */
    uint8_t  _reserved[7]; /* Reserved for future use */
    uint16_t _Unused16; /* Currently unused, reserved for future use */
    uint16_t _crc16; /* CRC16 of the error info for integrity checking */
} sErrorCompact_t;
#pragma pack( pop )
#endif
#ifndef SERROR_CODE_MESSAGE_PAIR
#define SERROR_CODE_MESSAGE_PAIR
/** 
 * @brief Structure that contains an error code and its corresponding message.
 */
typedef struct
{
    uint16_t _errorCode; /* The error code */
    char _errorMessage[ MAX_ERROR_MESSAGE_LENGTH_BYTES ]; /* The corresponding error message */  
} sErrorCodeMessagePair_t;
#endif // SERROR_CODE_MESSAGE_PAIR
/**************************** HELPER MACROS ***********************************/
#ifndef BLANK_ERROR_STRUCT
#define BLANK_ERROR_STRUCT { \
    ._errorCode = ERROR_NONE, \
    ._fileModuleEnum = 0U, \
    ._lineNumber = 0, \
    ._flags = 0, \
    ._reserved = { 0 }, \
    ._Unused16 = 0, \
    ._crc16 = 0 \
} 
#endif

/**
 * @brief Macro to create an error structure with the given parameters. Does not
 * store the error in the error driver.
 * @param errorCode The error code for this error
 * @param errorMessage A message describing the error
 * @return a compact error structure containing the error information.
 */
#define CREATE_ERROR( errorCode, errorMessage ) \
    createErrorCompact( errorCode, MODULE_ID, __LINE__, false, errorMessage, moduleName)
/**
 * @brief Macro to create an error structure with the given parameters and store it in the error driver.
 * @param errorCode The error code for this error  
 * @param errorMessage A message describing the error
 * @return a compact error structure containing the error information.
 */
#define CREATE_STORE_ERROR( errorCode, autoStoreError, errorMessage ) \
    createErrorCompact( errorCode, MODULE_ID, __LINE__, autoStoreError, errorMessage, moduleName )
/*****************************Public Interface Functions ***********************/
/**
 * @brief Function to store an error with the error driver. 
 *        This will store the error information in a circular buffer for later retrieval.
 * 
 * @param errorCode The error code for this error
 * @param errorInfo a pointer to an sErrorCompact_t structure containing the error information to log.
 * @return an error info structure indicating success or failure.
 * 
 */
extern sErrorCompact_t storeError( sErrorCompact_t * const errorInfo,
                                   uint8_t const * const errorMessage, 
                                   uint8_t const * const moduleName  );

/**
 * @brief Function to create an ERROR structure given an error code, line number, filename, 
 *        and an error message.
 * @param errorMessage A message describing the error
 * @param fileModuleEnum The module ID where the error occurred.
 * @param autoStoreError Whether or not to automatically store the error in the circular buffer.
 * @param moduleName The name of the module where the error occurred.
 * @param lineNumber The line number where the error occurred
 */
extern sErrorCompact_t createErrorCompact( uint16_t errorCode, 
                                           uint16_t fileModuleEnum, 
                                           uint16_t lineNumber,
                                           bool autoStoreError,
                                           uint8_t const * const errorMessage, 
                                           uint8_t const * const moduleName );

/** 
 * @brief Function to retrieve the last error that was logged with the error driver. This will return the most recent error information that was logged.
 * @return sErrorCompact_t structure containing the error information for the last error that was logged.
*/
extern sErrorCompact_t getLastError( void );

/**
 * @brief Function to retrieve an error by its index in the error log.
 * @param index The index of the error to retrieve
 * @return sErrorCompact_t structure containing the error information for the specified index.
 */
extern sErrorCompact_t getErrorByIndex( uint16_t * const index,  sErrorCompact_t * const errorInfo ); 

/**
 * @brief Function to get the number of errors currently stored in the error log.
 * @param errorCount Pointer to a uint16_t variable to store the number of errors.
 * @return An error if one occurrred.
 */
extern sErrorCompact_t getErrorCount( uint16_t * const errorCount );

/**
 * @brief Function to clear all errors from the error log.
 */
extern void clearErrors( void );

/**
 * @brief Function to print all errors currently stored in the error log.
 */
extern void printAllErrors( void );

/**
 * @brief Function to initialize the error driver. This should be called before any other functions are used.
 * @param readMemory Pointer to a function that reads memory for the error driver. 
 *                   This is used to read the error information from the circular buffer.
 * @param writeMemory Pointer to a function for writing memory for the error driver.
 * @param logCallback Pointer to a function for logging errors. This is used to log errors to the logging driver.
 * @param
 * @param memoryAddress The starting address of the memory to be used by the error driver.
 * @param memorySizeInBytes Size of the memory in bytes
 * @return sErrorCompact_t structure containing the error information if an error occurred.
 */
extern sErrorCompact_t initErrorDriver( readMemoryFunctionPtr_t readMemory,
                                        writeMemoryFunctionPtr_t writeMemory,
                                        logCallback_t logCallback,                                        
                                        calculateCRC16FunctionPtr_t calculateCRC16Function,
                                        uint32_t memoryAddress,
                                        uint16_t memorySizeInBytes );

#if ( ( ERROR_MESSAGE_FULL == DEF_TRUE ) || ( LOG_FULL_ERROR_MESSAGE == DEF_TRUE ) )
/**
 * @brief Function to get the error message corresponding error error code.
 * @param errorCode The common error code to get the message for.
 * @param errorMessage Pointer to a buffer to store the error message.
 * @returns An error struct with ERROR_NONE if successful, or an error struct with an error code if unsuccessful.
 */
extern sErrorCompact_t getErrorMessageFromErrorCode( sErrorCodeMessagePair_t const * const errorCodeMessagePairs,
                                              uint16_t const lastErrorCode,
                                              uint16_t const errorCode,
                                              bool const autoStoreError,
                                              uint8_t const * errorMessage );
#endif
/**
 * @brief Enter a watchdog-friendly spin loop after a debug assert failure.
 * @param expression The failed expression string.
 * @param fileName The source file where the assert failed.
 * @param lineNumber The source line where the assert failed.
 */
extern void cErrorDriverDebugAssertSpin( char const * const expression,
                                         char const * const fileName,
                                         unsigned int lineNumber );

/**
 * @brief Helper function to access the driver information in a ecapsulated way. 
 *        This is used to get the module ID, version string, and other information 
 *         about the error driver.
 * @return sCommonDriverAccessorStruct_t structure containing the error driver information.
 */
extern sCommonDriverAccessorStruct_t const * const getErrorDriverInfoAccessors( void );

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* C_ERROR_DRIVER_PUB_H */
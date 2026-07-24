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

/*************************************** Typedefs **************************************/
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

/** 
 * @brief Structure that contains an error code and its corresponding message.
 */
typedef struct
{
    uint16_t _errorCode; /* The error code */
    char _errorMessage[ MAX_ERROR_MESSAGE_LENGTH_BYTES ]; /* The corresponding error message */  
} sErrorCodeMessagePair_t;


/**************************** HELPER MACROS ***********************************/
#if ( ERROR_MESSAGE_FULL == DEF_TRUE )
#define BLANK_ERROR_STRUCT {{ ERROR_NONE, 0, 0, 0, { 0 }, 0, 0 }, { { 0 }, { 0 } } }
#else
#define BLANK_ERROR_STRUCT {{ ERROR_NONE, 0, 0, 0, { 0 }, 0, 0 } }
#endif

#define CREATE_ERROR( errorCode, errorMessage ) \
    createErrorInfo( errorCode, THIS->_driverControl._driverInfo._moduleID, __LINE__ , errorMessage, THIS->_driverControl._driverInfo._moduleName )

/**
 * @brief Function to log an error with the error driver. 
 *        This will store the error information in a circular buffer for later retrieval.
 * 
 * @param errorCode The error code for this error
 * @param errorInfo a pointer to an sErrorInfo_t structure containing the error information to log.
 * @return an error info structure indicating success or failure.
 * 
 */
extern sErrorInfo_t logError( sErrorInfo_t * const errorInfo );

/**
 * @brief Function to create an ERROR structure given an error code, line number, filename, 
 *        and an error message.
 * @param errorMessage A message describing the error
 * @param fileModuleEnum The module ID where the error occurred.
 * @param moduleName The name of the module where the error occurred.
 * @param lineNumber The line number where the error occurred
 */
extern sErrorInfo_t createErrorInfo( uint16_t errorCode, 
                                     uint16_t fileModuleEnum, 
                                     uint16_t lineNumber, 
                                     uint8_t const * const errorMessage, 
                                     uint8_t const * const moduleName );

/** 
 * @brief Function to retrieve the last error that was logged with the error driver. This will return the most recent error information that was logged.
 * @return sErrorInfo_t structure containing the error information for the last error that was logged.
*/
extern sErrorInfo_t getLastError( void );

/**
 * @brief Function to retrieve an error by its index in the error log.
 * @param index The index of the error to retrieve
 * @return sErrorInfo_t structure containing the error information for the specified index.
 */
extern sErrorInfo_t getErrorByIndex( uint16_t * const index,  sErrorInfo_t * const errorInfo ); 

/**
 * @brief Function to get the number of errors currently stored in the error log.
 * @param errorCount Pointer to a uint16_t variable to store the number of errors.
 * @return An error if one occurrred.
 */
extern sErrorInfo_t getErrorCount( uint16_t * const errorCount );

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
 * @param memoryAddress The starting address of the memory to be used by the error driver.
 * @param memorySizeInBytes Size of the memory in bytes
 * @return sErrorInfo_t structure containing the error information if an error occurred.
 */
extern sErrorInfo_t initErrorDriver( readMemoryFunctionPtr_t readMemory, 
                                     writeMemoryFunctionPtr_t writeMemory,
                                     uint32_t memoryAddress,
                                     uint16_t memorySizeInBytes );



/**
 * @brief Function to register a module with the error driver.
 * @param moduleName The name of the module to register
 * @param moduleId The ID of the module to register
 * @return an error uint16_t code if successful
 */
extern uint16_t registerModule( const char * moduleName, uint16_t * const moduleId );

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
 * @brief Function to get the error driver information. This will return a structure containing the error driver information.
 * @return sCommonDriverAccessorStruct_t structure containing the error driver information.
 */
extern sCommonDriverAccessorStruct_t const * const getErrorDriverInfo( void );

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* C_ERROR_DRIVER_PUB_H */
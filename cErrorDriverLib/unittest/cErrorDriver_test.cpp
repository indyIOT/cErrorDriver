/** ***********************************************
 * @file cErrorDriver_test.cpp
 * @brief Unit tests for cErrorDriver.c
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <cstdarg>   // va_list, va_start, va_end
#include <cstdio>    // vsnprintf
#include "commonMacros.h"
#include "commonTypes.h"
#include "../publicInclude/cErrorDriverPub.h"
#include "../publicInclude/cErrorDriverConfig.h"

namespace 
{
    using namespace std;
    constexpr std::size_t kFakeMemoryWords = 256U;
    std::uint32_t gFakeMemory[kFakeMemoryWords] = { 0U };

    std::uint16_t fakeReadMemory( std::uint32_t address, std::uint8_t * const readBuffer, std::size_t readSize )
    {
        std::uint16_t returnValue = 0U;
        if( address < ( kFakeMemoryWords * sizeof( std::uint32_t ) ) )
        {
            if( readBuffer != nullptr && readSize != 0U )
            {
                if( readSize + address > ( kFakeMemoryWords * sizeof( std::uint32_t ) ) )
                {
                    readSize = ( kFakeMemoryWords * sizeof( std::uint32_t ) ) - address;
                }
                std::memcpy( readBuffer, reinterpret_cast<std::uint8_t*>( &gFakeMemory[address] ), readSize );
                returnValue = static_cast<std::uint16_t>(readSize);
            }
        }

        return returnValue;
    }

    std::uint16_t fakeWriteMemory( std::uint32_t address, std::uint8_t * data, std::size_t writeLength  )
    {
        std::uint16_t returnValue = 0U;
        if( address < ( kFakeMemoryWords * sizeof( std::uint32_t ) ) )
        {
            if( writeLength + address > ( kFakeMemoryWords * sizeof( std::uint32_t ) ) )
            {
                writeLength = ( kFakeMemoryWords * sizeof( std::uint32_t ) ) - address;
            }

            std::memcpy( &gFakeMemory[address], data, writeLength );
            returnValue = static_cast<std::uint16_t>(writeLength);
        }
        return returnValue;
    }

    sErrorCompact_t fakeLogCallback( uint16_t moduleId,
                                     uint16_t line,
                                     eLoggingType_t type,
                                     const char *message, ... )
    {
        sErrorCompact_t retValue = { 0 };
        char buffer[256];

        va_list args;
        va_start( args, message );          // start reading args right after 'message'
        vsnprintf( buffer, sizeof(buffer), message, args );  // does the %d/%s/etc. substitution
        va_end( args );

        cout << "[Module " << moduleId << ", Line " << line
            << ", Type " << type << "] " << buffer << endl;

    
        return retValue;

    }

    std::uint16_t fakeCalculateCRC16( sCRC16Config_t config, std::uint8_t const * const buffer, std::uint16_t const length )
    {
        (void)config;
        (void)buffer;
        (void)length;

        return 0xAA55; // Return a dummy CRC value for testing
    }

} // namespace

/** Going to test the initialization function first 
 * initErrorDriver( readMemoryFunctionPtr_t readMemory,
                                        writeMemoryFunctionPtr_t writeMemory,
                                        logCallback_t logCallback,
                                        calculateCRC16FunctionPtr_t calculateCRC16Function,
                                        uint32_t memoryAddress,
                                        uint16_t memorySizeInBytes )
*/
/**
 * @brief Test case for initializing the error driver with valid parameters.
 */
TEST( cErrorDriver, initErrorDriver )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );
}

/**
 * @brief Test case for initializing the error driver with NULL parameters.
 */
TEST( cErrorDriver, initNULLReadMemory )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)0U,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

/**
 * @brief Test case for initializing the error driver with NULL parameters.
 */
TEST( cErrorDriver, initNULLWriteMemory )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)0U,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

/**
 * @brief Test case for initializing the error driver with NULL parameters.
 */
TEST( cErrorDriver, initNullCalculateCRC16 )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)0U,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

/**
 * @brief Test case for initializing the error driver with NULL parameters.
 * A Null Log callback is not an error just no logging will occur.
 */
TEST( cErrorDriver, initNullLogCallback )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)0U,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );
}

/**
 * @brief Test initialiazing an already initalized driver.
 * 
 */
TEST( cErrorDriver, initAlreadyInitializedDriver )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );

    errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_ALREADY_INITIALIZED, errorInfo._errorCode );
}

/**
 * Testing the createErrorCompact function.
 * createErrorCompact( uint16_t errorCode,
 *                    uint16_t fileModuleEnum,
 *                   uint16_t lineNumber,
 *                  bool autoStoreError,
 *                 uint8_t const * const errorMessage,
 *                uint8_t const * const moduleName )
 * 
 */
TEST( cErrorDriver, createErrorCompactCommonError )
{
    uint32_t testErrorDetails;
    uint16_t moduleNumber = 0x5a5a;
    uint16_t lineNumber = 42U;
    
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        sizeof( gFakeMemory )
    );
    
    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );    
    errorInfo = createErrorCompact( ERROR_INVALID_PARAMETER, 
         1U, 42U, true, (uint8_t const * const)"Test Error Message", 
         (uint8_t const * const)"TestModule" );
    testErrorDetails = ( static_cast<uint32_t>(moduleNumber) << 16U ) | ERROR_INVALID_PARAMETER;

    EXPECT_EQ( ERROR_INVALID_PARAMETER, errorInfo._errorCode );
    EXPECT_EQ( testErrorDetails, errorInfo._errorDetails );
    EXPECT_EQ( lineNumber, errorInfo._lineNumber );
    EXPECT_EQ( 0xAA55, errorInfo._crc16 );
}

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
#include <cstring>   // memcpy, memset
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include "commonMacros.h"
#include "commonTypes.h"
#include "../publicInclude/cErrorDriverPub.h"
#include "../publicInclude/cErrorDriverConfig.h"

using namespace std;

namespace
{
    constexpr size_t kFakeMemoryWords = 256U;
    uint32_t gFakeMemory[kFakeMemoryWords] = { 0U };

    /** @brief Known-stable identifiers mirrored from cErrorDriver.c (private #define/static there). */
    constexpr uint16_t kErrorDriverModuleId = 31905U;
    const char * const kErrorDriverModuleName = "cErrorDriver";

    uint16_t fakeReadMemory( uint32_t address, uint8_t * const readBuffer, size_t readSize )
    {
        uint16_t returnValue = 0U;
        if( address < ( kFakeMemoryWords * sizeof( uint32_t ) ) )
        {
            if( readBuffer != nullptr && readSize != 0U )
            {
                if( readSize + address > ( kFakeMemoryWords * sizeof( uint32_t ) ) )
                {
                    readSize = ( kFakeMemoryWords * sizeof( uint32_t ) ) - address;
                }
                memcpy( readBuffer, reinterpret_cast<uint8_t*>( &gFakeMemory[0] ) + address, readSize );
                returnValue = static_cast<uint16_t>( readSize );
            }
        }
        return returnValue;
    }

    /**
     * @brief Fake write function matching writeMemoryFunctionPtr_t's documented contract:
     *        returns ERROR_NONE (0) on success, non-zero on failure. The previous fake
     *        returned the byte count on success, which cErrorDriver.c's write-failure
     *        check (writeCheck != ERROR_NONE) misread as a failure on every successful write.
     */
    uint16_t fakeWriteMemory( uint32_t const address, uint8_t * const data, size_t const writeLength, bool const verifyWrite )
    {
        (void)verifyWrite;
        uint16_t returnValue = 1U; // non-zero => failure
        if( ( data != nullptr ) &&
            ( writeLength != 0U ) &&
            ( address < ( kFakeMemoryWords * sizeof( uint32_t ) ) ) &&
            ( ( address + writeLength ) <= ( kFakeMemoryWords * sizeof( uint32_t ) ) ) )
        {
            memcpy( reinterpret_cast<uint8_t*>( &gFakeMemory[0] ) + address, data, writeLength );
            returnValue = 0U; // ERROR_NONE => success
        }
        return returnValue;
    }

    /** @brief Always fails, regardless of input. Used to exercise the write-failure logging path. */
    uint16_t fakeWriteMemoryAlwaysFails( uint32_t const address, uint8_t * const data, size_t const writeLength, bool const verifyWrite )
    {
        (void)address;
        (void)data;
        (void)writeLength;
        (void)verifyWrite;
        return 0xACU; // arbitrary non-zero failure code
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
        vsnprintf( buffer, sizeof( buffer ), message, args );  // does the %d/%s/etc. substitution
        va_end( args );

        cout << "[Module " << moduleId << ", Line " << line
             << ", Type " << type << "] " << buffer << endl;

        return retValue;
    }

    uint16_t fakeCalculateCRC16( sCRC16Config_t config, void const * const buffer, size_t const length )
    {
        (void)config;
        (void)buffer;
        (void)length;
        return 0xAA55U; // fixed dummy CRC for testing
    }

    /**
     * @brief Fixture that gives every test a freshly-uninitialized driver and a
     *        zeroed fake memory backing store. Relies on resetErrorDriverForTest(),
     *        a UNIT_TESTS-only hook (see cErrorDriverPub.h), because the driver's
     *        control struct is a file-scope static singleton with no production
     *        way to de-initialize it.
     */
    class ErrorDriverTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            resetErrorDriverForTest();
            memset( gFakeMemory, 0, sizeof( gFakeMemory ) );
        }

        sErrorCompact_t initValid( logCallback_t logCallback = (logCallback_t)&fakeLogCallback,
                                    writeMemoryFunctionPtr_t writeMemory = (writeMemoryFunctionPtr_t)&fakeWriteMemory )
        {
            return initErrorDriver(
                (readMemoryFunctionPtr_t)&fakeReadMemory,
                writeMemory,
                logCallback,
                (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
                0U,
                static_cast<uint16_t>( sizeof( gFakeMemory ) ) );
        }
    };

} // namespace

/*****************************************************************************
 * initErrorDriver()
 ****************************************************************************/

TEST_F( ErrorDriverTest, InitWithValidParametersSucceeds )
{
    sErrorCompact_t errorInfo = initValid();
    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );
}

TEST_F( ErrorDriverTest, InitWithNullReadMemoryFailsWithNullPointer )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)0U,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        static_cast<uint16_t>( sizeof( gFakeMemory ) ) );

    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

TEST_F( ErrorDriverTest, InitWithNullWriteMemoryFailsWithNullPointer )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)0U,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        static_cast<uint16_t>( sizeof( gFakeMemory ) ) );

    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

TEST_F( ErrorDriverTest, InitWithNullCalculateCRC16FailsWithNullPointer )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)0U,
        0U,
        static_cast<uint16_t>( sizeof( gFakeMemory ) ) );

    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

TEST_F( ErrorDriverTest, InitWithMemoryTooSmallFailsWithInvalidParameter )
{
    sErrorCompact_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        (logCallback_t)&fakeLogCallback,
        (calculateCRC16FunctionPtr_t)&fakeCalculateCRC16,
        0U,
        static_cast<uint16_t>( sizeof( sErrorCompact_t ) - 1U ) );

    EXPECT_EQ( ERROR_INVALID_PARAMETER, errorInfo._errorCode );
}

TEST_F( ErrorDriverTest, InitWithNullLogCallbackSucceeds )
{
    // A null log callback is not an error -- logging is simply skipped.
    sErrorCompact_t errorInfo = initValid( (logCallback_t)0U );
    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );
}

TEST_F( ErrorDriverTest, InitCalledTwiceReturnsAlreadyInitialized )
{
    sErrorCompact_t firstInit = initValid();
    ASSERT_EQ( ERROR_NONE, firstInit._errorCode );

    sErrorCompact_t secondInit = initValid();
    EXPECT_EQ( ERROR_ALREADY_INITIALIZED, secondInit._errorCode );
}

/*****************************************************************************
 * createErrorCompact()
 ****************************************************************************/

TEST_F( ErrorDriverTest, CreateErrorCompactWithErrorNoneIsANoOp )
{
    // ERROR_NONE short-circuits the whole function; the struct stays blank.
    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_NONE, 7U, 99U, true,
        (uint8_t const * const)"unused", (uint8_t const * const)"unused" );

    EXPECT_EQ( ERROR_NONE, errorInfo._errorCode );
    EXPECT_EQ( 0U, errorInfo._fileModuleEnum );
    EXPECT_EQ( 0U, errorInfo._lineNumber );
    EXPECT_EQ( 0U, errorInfo._crc16 );
}

TEST_F( ErrorDriverTest, CreateErrorCompactBeforeInitSetsFieldsButSkipsCrcAndLogging )
{
    // Driver is uninitialized (fresh from SetUp). createErrorCompact should still
    // fill in the basic fields without touching the (null) CRC/log/write callbacks.
    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_TIMEOUT, 7U, 99U, true,
        (uint8_t const * const)"Not initialized yet", (uint8_t const * const)"TestModule" );

    EXPECT_EQ( ERROR_TIMEOUT, errorInfo._errorCode );
    EXPECT_EQ( 7U, errorInfo._fileModuleEnum );
    EXPECT_EQ( 99U, errorInfo._lineNumber );
    EXPECT_EQ( 0U, errorInfo._crc16 ); // CRC step is gated on the driver being initialized
}

TEST_F( ErrorDriverTest, CreateErrorCompactAfterInitSetsCrc16FromCallback )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );

    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_INVALID_STATE, 2U, 10U, false,
        (uint8_t const * const)"CRC should be set", (uint8_t const * const)"TestModule" );

    EXPECT_EQ( 0xAA55U, errorInfo._crc16 );
}

TEST_F( ErrorDriverTest, CreateErrorCompactPacksErrorCodeAndModuleIntoErrorDetails )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );

    constexpr uint16_t moduleEnum = 0x5A5AU;
    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_INVALID_PARAMETER, moduleEnum, 42U, false,
        (uint8_t const * const)"Test Error Message", (uint8_t const * const)"TestModule" );

    const uint32_t expectedDetails = ( static_cast<uint32_t>( moduleEnum ) << 16U ) | ERROR_INVALID_PARAMETER;

    EXPECT_EQ( ERROR_INVALID_PARAMETER, errorInfo._errorCode );
    EXPECT_EQ( moduleEnum, errorInfo._fileModuleEnum );
    EXPECT_EQ( 42U, errorInfo._lineNumber );
    EXPECT_EQ( expectedDetails, errorInfo._errorDetails );
}

TEST_F( ErrorDriverTest, CreateErrorCompactWithAutoStoreWritesToMemory )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );

    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_TIMEOUT, 3U, 55U, true,
        (uint8_t const * const)"Disk timeout", (uint8_t const * const)"StorageModule" );
    ASSERT_EQ( ERROR_TIMEOUT, errorInfo._errorCode );

    // The driver writes sErrorInfo_t (compact struct first) to memoryAddress 0.
    sErrorCompact_t writtenCompact;
    memcpy( &writtenCompact, gFakeMemory, sizeof( writtenCompact ) );

    EXPECT_EQ( ERROR_TIMEOUT, writtenCompact._errorCode );
    EXPECT_EQ( 3U, writtenCompact._fileModuleEnum );
    EXPECT_EQ( 55U, writtenCompact._lineNumber );
    EXPECT_EQ( 0xAA55U, writtenCompact._crc16 );
}

TEST_F( ErrorDriverTest, CreateErrorCompactWithoutAutoStoreDoesNotWriteToMemory )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );

    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_TIMEOUT, 3U, 55U, false,
        (uint8_t const * const)"Should not be stored", (uint8_t const * const)"StorageModule" );
    ASSERT_EQ( ERROR_TIMEOUT, errorInfo._errorCode );

    uint8_t allZero[sizeof( gFakeMemory )] = { 0 };
    EXPECT_EQ( 0, memcmp( allZero, gFakeMemory, sizeof( gFakeMemory ) ) );
}

TEST_F( ErrorDriverTest, CreateErrorCompactWriteFailureLogsCriticalMessage )
{
    ASSERT_EQ( ERROR_NONE, initValid( (logCallback_t)&fakeLogCallback,
                                       (writeMemoryFunctionPtr_t)&fakeWriteMemoryAlwaysFails )._errorCode );

    ::testing::internal::CaptureStdout();
    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_TIMEOUT, 3U, 55U, true,
        (uint8_t const * const)"Write will fail", (uint8_t const * const)"StorageModule" );
    const string output = ::testing::internal::GetCapturedStdout();

    // The error is still returned to the caller even though persisting it failed.
    EXPECT_EQ( ERROR_TIMEOUT, errorInfo._errorCode );
    EXPECT_NE( string::npos, output.find( "Failed to write error info to memory" ) );
}

TEST_F( ErrorDriverTest, CreateErrorCompactLogsFormattedErrorDetails )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );

    ::testing::internal::CaptureStdout();
    (void)createErrorCompact(
        ERROR_INVALID_PARAMETER, 1U, 42U, false,
        (uint8_t const * const)"Test Error Message", (uint8_t const * const)"TestModule" );
    const string output = ::testing::internal::GetCapturedStdout();

    EXPECT_NE( string::npos, output.find( "Error Code: 0x0001" ) );
    EXPECT_NE( string::npos, output.find( "Line: 42" ) );
    EXPECT_NE( string::npos, output.find( "Message: Test Error Message" ) );
}

TEST_F( ErrorDriverTest, CreateErrorCompactWithNullMessageUsesDefaultMessage )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );

    ::testing::internal::CaptureStdout();
    (void)createErrorCompact(
        ERROR_UNKNOWN, 1U, 1U, false,
        nullptr, (uint8_t const * const)"TestModule" );
    const string output = ::testing::internal::GetCapturedStdout();

    EXPECT_NE( string::npos, output.find( "No error message available." ) );
}

TEST_F( ErrorDriverTest, CreateErrorCompactWithNoLogCallbackDoesNotCrash )
{
    ASSERT_EQ( ERROR_NONE, initValid( (logCallback_t)0U )._errorCode );

    sErrorCompact_t errorInfo = createErrorCompact(
        ERROR_UNKNOWN, 1U, 1U, false,
        (uint8_t const * const)"No logger installed", (uint8_t const * const)"TestModule" );

    EXPECT_EQ( ERROR_UNKNOWN, errorInfo._errorCode );
}

/*****************************************************************************
 * getErrorMessageFromErrorCode()
 ****************************************************************************/

TEST_F( ErrorDriverTest, GetErrorMessageFromErrorCodeValidCodeReturnsErrorNone )
{
    sErrorCompact_t result = getErrorMessageFromErrorCode(
        nullptr, END_OF_COMMON_ERRORS, ERROR_INVALID_PARAMETER, false, nullptr );

    EXPECT_EQ( ERROR_NONE, result._errorCode );
}

TEST_F( ErrorDriverTest, GetErrorMessageFromErrorCodeOutOfRangeReturnsInvalidParameter )
{
    // errorCode >= lastErrorCode is out of range.
    sErrorCompact_t result = getErrorMessageFromErrorCode(
        nullptr, END_OF_COMMON_ERRORS, END_OF_COMMON_ERRORS, false, nullptr );

    EXPECT_EQ( ERROR_INVALID_PARAMETER, result._errorCode );
}
// NOTE: getErrorMessageFromErrorCode's `errorMessage` parameter is passed by value
// (uint8_t const *, not a pointer-to-pointer/output buffer), and the function
// immediately overwrites its own local copy before doing anything with it. The
// resolved message string is therefore never actually visible to the caller --
// only the pass/fail return code is observable. That looks like a pre-existing
// bug in cErrorDriver.c rather than a test issue; flagging it here rather than
// silently working around it.

/*****************************************************************************
 * getErrorDriverInfoAccessors()
 ****************************************************************************/

TEST_F( ErrorDriverTest, AccessorsReturnNonNullStruct )
{
    sCommonDriverAccessorStruct_t const * const accessors = getErrorDriverInfoAccessors();
    ASSERT_NE( nullptr, accessors );
    EXPECT_NE( nullptr, accessors->getModuleIdFunction );
    EXPECT_NE( nullptr, accessors->getModuleVersionStringFunction );
    EXPECT_NE( nullptr, accessors->getModuleNameFunction );
    EXPECT_NE( nullptr, accessors->getModuleVersionFunction );
    EXPECT_NE( nullptr, accessors->isDriverInitializedFunction );
}

TEST_F( ErrorDriverTest, AccessorsReportModuleIdAndName )
{
    sCommonDriverAccessorStruct_t const * const accessors = getErrorDriverInfoAccessors();

    EXPECT_EQ( kErrorDriverModuleId, accessors->getModuleIdFunction() );
    EXPECT_STREQ( kErrorDriverModuleName, reinterpret_cast<char const *>( accessors->getModuleNameFunction() ) );
}

TEST_F( ErrorDriverTest, AccessorsReportVersionInfo )
{
    sCommonDriverAccessorStruct_t const * const accessors = getErrorDriverInfoAccessors();

    uint8_t const * const versionString = accessors->getModuleVersionStringFunction();
    ASSERT_NE( nullptr, versionString );
    EXPECT_GT( strlen( reinterpret_cast<char const *>( versionString ) ), 0U );

    sCommonVersionStruct_t version = accessors->getModuleVersionFunction();
    EXPECT_EQ( STATIC_LIBRARY_BUILD, version._buildType ); // COMPILE_ERROR_DRIVER_LIBRARY_STATIC is ON
}

TEST_F( ErrorDriverTest, AccessorsIsDriverInitializedReflectsState )
{
    sCommonDriverAccessorStruct_t const * const accessors = getErrorDriverInfoAccessors();

    EXPECT_FALSE( accessors->isDriverInitializedFunction() );
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );
    EXPECT_TRUE( accessors->isDriverInitializedFunction() );
}

/*****************************************************************************
 * cErrorDriverDebugAssertSpin()
 *
 * This function is an intentional infinite loop meant to hang the device until
 * a watchdog resets it (or a debugger manually clears its internal static flag,
 * which isn't reachable from outside the function). It can never be called
 * and "finish" in a normal test. Instead we assert on the one behavior that
 * *is* testable: it must never return. We run it on a background thread with a
 * short timeout and confirm the thread never signals completion, then detach
 * that thread (it will spin for the rest of the process's life by design).
 ****************************************************************************/

TEST( ErrorDriverDebugAssertSpin, NeverReturnsWithinTimeout )
{
    std::promise<void> returned;
    std::future<void> returnedFuture = returned.get_future();

    std::thread spinThread( [&returned]()
    {
        cErrorDriverDebugAssertSpin( "1 == 2", "fake_test_file.c", 123U );
        returned.set_value(); // Should never execute.
    } );

    const std::future_status status = returnedFuture.wait_for( std::chrono::milliseconds( 200 ) );

    EXPECT_EQ( std::future_status::timeout, status )
        << "cErrorDriverDebugAssertSpin() returned, but it is documented to spin "
           "forever until a debugger clears the internal 'keepSpinning' flag.";

    spinThread.detach();
}

TEST( ErrorDriverDebugAssertSpin, AssertMacroDoesNotSpinWhenExpressionIsTrue )
{
    // AG_DEBUG_ASSERT only calls cErrorDriverDebugAssertSpin() when the expression
    // is false. If this ever regressed to spin unconditionally, this test would
    // hang instead of completing -- a visible failure in any CI run.
    AG_DEBUG_ASSERT( 1 == 1 );
    SUCCEED();
}

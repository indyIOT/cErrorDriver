/** **************************************************************
 * @file: main.c
 * @brief Main entry point of the test application.
 * @author Anthony Garza
 * @copyright Copyright 2023 All Rights Reserved.
****************************************************************/

#include <stdio.h>
#include "commonMacros.h"
#include "cErrorDriverPub.h"

#define FAKE_MEMORY_WORDS 256U
static uint32_t gFakeMemory[FAKE_MEMORY_WORDS];

static uint16_t fakeReadMemory( uint32_t address, uint8_t * const readValue, size_t readSize )
{
    if( address >= FAKE_MEMORY_WORDS )
    {
        return -1;
    }
    else if( readValue == NULL )
    {
        return -2;
    }
    else if( readSize == 0 )
    {
        return -3;
    }
    else if( ( address + readSize ) > FAKE_MEMORY_WORDS )
    {
        return -4;
    }
    else
    {
        (void)memcpy( readValue, &gFakeMemory[address], readSize );
    }

    return 0;
}

static uint16_t fakeWriteMemory( uint32_t address, uint8_t * value, size_t writeSize )
{
    if( address >= FAKE_MEMORY_WORDS )
    {
        return -1;
    }
    else if( value == NULL )
    {
        return -2;
    }
    else if( writeSize == 0 )
    {
        return -3;
    }
    else if( ( address + writeSize ) > FAKE_MEMORY_WORDS )
    {
        return -4;
    }
    else
    {
        (void)memcpy( &gFakeMemory[address], value, writeSize );
    }
    return 0;
}


/**
 * @brief Program Main entry for testing libraries.
 * 
 * @return int 
 */
int main( void )
{
    int retValue = ERROR_NONE;
    sErrorInfo_t errorInfo = initErrorDriver(
        (readMemoryFunctionPtr_t)&fakeReadMemory,
        (writeMemoryFunctionPtr_t)&fakeWriteMemory,
        0U,
        sizeof( gFakeMemory )
    );
    return( retValue );
}

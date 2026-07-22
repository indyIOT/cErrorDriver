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
        return 0U;
    }

    return gFakeMemory[address];
}

static uint16_t fakeWriteMemory( uint32_t address, uint8_t * value, size_t writSize )
{
    if( address >= kFakeMemoryWords )
    {
        return false;
    }

    gFakeMemory[address] = value;
    return true;
}


/**
 * @brief Program Main entry for testing libraries.
 * 
 * @return int 
 */
int main( void )
{
    int retValue = ERROR_NONE;
    sErrorInfo_t
    return( retValue );
}

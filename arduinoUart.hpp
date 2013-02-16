#ifndef UART_CONTROLLER_HPP
#define UART_CONTROLLER_HPP

#include <iostream>

#ifdef _WIN32
// Windows specific include files
#include "windows.h"
#elif defined __linux__
// Linux specific include files
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitionss
#endif

using namespace std;

class arduinoUart {
    public:
        arduinoUart();
        arduinoUart( const char *com_name, int baud_rate_req = 9600 ); // ex com_name = "COM5" or "/dev/ttyUSB0"
        ~arduinoUart();
        bool empty_read_buffer();
        bool read( unsigned char *rdata, int num_bytes);
        bool write( unsigned char *wdata, int num_bytes );
        void set_verbose( bool v_req );
        void set_verbose( int v_level_req );
// Debugging function
        bool read_byte( unsigned char *buffer );
        bool write_byte( unsigned char buffer );
    private:
        bool init( const char *com_name );
#ifdef _WIN32
        HANDLE hSerial;
#elif defined __linux__
        int fd;
#endif
        int baud_rate; // not implemented
        bool verbose;
};

#endif


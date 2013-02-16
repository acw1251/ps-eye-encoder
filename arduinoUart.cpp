#include "arduinoUart.hpp"

using namespace std;

arduinoUart::arduinoUart() {
#ifdef DEBUG
    this->verbose = true;
#else
    this->verbose = false;
#endif
#ifdef _WIN32
    this->init( "COM5" );
#elif defined __linux__
    this->init( "/dev/ttyUSB0" );
#endif
    this->baud_rate = 9600; // default
}

arduinoUart::arduinoUart( const char* com_name, int baud_rate_req ) {
#ifdef DEBUG
    this->verbose = true;
#else
    this->verbose = false;
#endif
    this->baud_rate = baud_rate_req;

    this->init( com_name );
}

arduinoUart::~arduinoUart() {
#ifdef _WIN32
    CloseHandle(this->hSerial);
#elif defined __linux__
    close(fd);
#endif
    if( this->verbose ) cout << "arduinoUart::~arduinoUart : [INFO] arduinoUart destroyed successfully." << endl;
}


bool arduinoUart::init( const char* com_name ) {
#ifdef _WIN32
    this->hSerial = CreateFile(   com_name,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            0);

    if(this->hSerial==INVALID_HANDLE_VALUE){
        if(GetLastError()==ERROR_FILE_NOT_FOUND){
            cout << "arduinoUart::init : [ERROR] Serial port does not exist." << endl;
            return false;
        }
        cout << "arduinoUart::init : [ERROR] Some other error occurred." << endl;
        return false;
    }

    // configure comm settings
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(this->hSerial, &dcbSerialParams)) {
        cout << "arduinoUart::init : [ERROR] Error getting state." << endl;
        return false;
    }
    dcbSerialParams.BaudRate=CBR_9600;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    if(!SetCommState(this->hSerial, &dcbSerialParams)){
        cout << "arduinoUart::init : [ERROR] Error setting serial port state." << endl;
        return false;
    }

    // configure timeouts
    COMMTIMEOUTS timeouts={0};
    timeouts.ReadIntervalTimeout=50;
    timeouts.ReadTotalTimeoutConstant=50;
    timeouts.ReadTotalTimeoutMultiplier=10;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=10;
    if(!SetCommTimeouts(this->hSerial, &timeouts)){
        cout << "arduinoUart::init : [ERROR] SetCommTimeouts failed." << endl;
        return false;
    }
#elif defined __linux__
    fd = open( com_name, O_RDWR | O_NOCTTY | O_NDELAY );
    if( fd == -1 ) {
        cout << "arduinoUart::init : [ERROR] Could not open serial port for writing" << endl;
        return false;
    }
    struct termios options;
    tcgetattr(fd, &options);    /* Get the current options for the port */
    if( baud_rate == 9600 ) {
        cfsetispeed(&options, B9600);   /* Set the baud rates to 9600 */
        cfsetospeed(&options, B9600);
    } else if( baud_rate == 115200 ) {
        cfsetispeed(&options, B115200);   /* Set the baud rates to 115200 */
        cfsetospeed(&options, B115200);
    } else {
        cout << "arduinoUart::init : [ERROR] Unrecognized baud rate. (baud_rate = " << baud_rate << ")" << endl;
        throw -1;
    }
    options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
                        ONOCR | OFILL | OLCUC | OPOST);
                        // ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    options.c_lflag = 0;
    options.c_cflag |= CS8 | CSTOPB | CREAD | CLOCAL;

    tcsetattr(fd, TCSANOW, &options);   /* Set the new options for the port */
    tcflush(fd, TCIOFLUSH);
    tcflow(fd, TCOON);
#endif
    
    if(this->verbose) cout << "arduinoUart::init : [INFO] arduinoUart configured successfully." << endl;
    return true;
}





bool arduinoUart::empty_read_buffer( ) {

    if( this->verbose ) cout << "arduinoUart::empty_read_buffer : [INFO] Emptying read buffer... " << endl;

#ifdef _WIN32
    unsigned char szBuff[16];
    DWORD dwBytesRead = 0;
    do {
        ReadFile(this->hSerial, szBuff, 15, &dwBytesRead, NULL);
        if( this->verbose ) {
            cout << "Bytes read = " << dwBytesRead;
            for( int i = 0 ; i < dwBytesRead ; i++ ) {
                if( i == 0 ) cout << " (";
                cout << hex << showbase << (int) szBuff[i];
                if( i != (dwBytesRead - 1) ) cout << ", ";
                else cout << ")";
            }
            cout << endl;
        }
    } while( dwBytesRead != 0 );
#elif defined __linux__
    unsigned char buffer[256];
    sleep( 1 ); // sleep 1 second to allow for incoming data
    int ret = ::read( this->fd, buffer, 256);
    while( ret != -1 ) {
        if( this->verbose ) cout << "arduinoUart::empty_read_buffer : [INFO] Emptied " << ret << " bytes from the buffer." << endl;
        ret = ::read( this->fd, buffer, 256);
    }
#endif

    return true;
}



bool arduinoUart::read( unsigned char *rdata, int num_bytes ) {
    if( this->verbose ) cout << "arduinoUart::read : [INFO] Reading " << dec << num_bytes << " byte(s)." << endl;
#ifdef _WIN32
    if(!ReadFile(this->hSerial, rdata, num_bytes, &dwBytesRead, NULL)){
        cout << "arduinoUart::read : [ERROR] Reading error occurred." << endl;
        return false;
    }

    if( this->verbose ) {
        cout << "arduinoUart::read : [INFO] " << dec << dwBytesRead << " byte(s) read." << endl;
    }
#elif defined __linux__
    int bytes_read = 0;
    while( bytes_read < num_bytes ) {
        int ret = ::read( this->fd, rdata + bytes_read, num_bytes - bytes_read );
        if( ret > 0 ) {
            bytes_read += ret;
            if( this->verbose ) cout << "arduinoUart::read : [INFO] Read " << dec << bytes_read << " out of " << dec << num_bytes << " bytes." << endl;
        } else if( /* ret == -1 && */ errno == 11 ) {
            // Try again!
        } else {
            cout << "arduinoUart::read : [ERROR] Reading error occurred. Returned " << dec << ret << " (errno = " << dec << errno << ")" << endl;
            return false;
        }
        // 10 should be good enough
        // usleep((num_bytes * 10.0) * 1000000.0 / ((double) baud_rate));
    }
#endif
    // if( this->verbose ) cout << "Data read = " << hex << showbase << *rdata << endl;
    return true;
}



bool arduinoUart::write( unsigned char *wdata, int num_bytes ) {
    // Quick fix to limit writes to 2048 bytes
    if( num_bytes > 2048 ) {
        this->write( wdata, 2048 );
        return this->write( wdata+2048, num_bytes-2048 );
    }

    if( this->verbose ) cout << "arduinoUart::write : [INFO] Writing " << dec << num_bytes << " byte(s)." << endl;
#ifdef _WIN32
    DWORD dwBytesWritten = 0;
    if(!WriteFile(this->hSerial, wdata, num_bytes, &dwBytesWritten, NULL)){
        cout << "arduinoUart::write : [ERROR] Writing error occured." << endl;
        return false;
    }

    if( this->verbose ) {
        cout << "arduinoUart::write : [INFO] " << dec << dwBytesWritten << " byte(s) written." << endl;
    }
#elif defined __linux__
    int ret = ::write( fd, wdata, num_bytes );
    if( ret == -1 ) {
        cout << "arduinoUart::write : [ERROR] Writing error occured. Returned " << dec << ret << " (errno = " << dec << errno << ")" << endl;
        return false;
    } else if( ret != num_bytes ) {
        cout << "arduinoUart::write : [ERROR] Writing error occured. Only wrote " << dec << ret << " bytes instead of " << dec << num_bytes << endl;
        return false;
    }
    // 10 should be good enough
    usleep((num_bytes * 10.0) * 1000000.0 / ((double) baud_rate));
#endif
    return true;
}



void arduinoUart::set_verbose( bool v_req ) {
    this->verbose = v_req;
}

void arduinoUart::set_verbose( int v_level_req ) {
    this->verbose = (v_level_req > 0);
}

bool arduinoUart::read_byte( unsigned char* buffer ) {
#ifdef _WIN32
    cout << "arduinoUart::read_byte : [ERROR] Function not supported for Windows!" << endl;
    return false;
#elif defined __linux__
    // Wait a second
    sleep( 1 );
    // now read
    int bytes_read = ::read( this->fd, buffer, 1 );
    if( bytes_read == 1 ) {
        // all good
        return true;
    } else if( bytes_read == -1 && errno == 11 ) {
        // buffer probably empty
        return false;
    } else {
        // an actual error happened
        cout << "arduinoUart::read_byte : [ERROR] Reading error occurred. Returned " << dec << bytes_read << " (errno = " << dec << errno << ")" << endl;
        return false;
    }
#endif
}

bool arduinoUart::write_byte( unsigned char buffer ) {
#ifdef _WIN32
    cout << "arduinoUart::write_byte : [ERROR] Function not supported for Windows!" << endl;
    return false;
#elif defined __linux__
    // Wait a second
    // sleep( 1 );
    // now write
    int bytes_written = ::write( this->fd, &buffer, 1 );
    if( bytes_written == 1 ) {
        // all good
        return true;
    } else {
        // an actual error happened
        cout << "arduinoUart::write_byte : [ERROR] Reading error occurred. Returned " << dec << bytes_written << " (errno = " << dec << errno << ")" << endl;
        return false;
    }
#endif
}



#pragma once

#include <windows.h>
#include <string>

class CSerialPort
{
public:

   typedef enum {
      ONE,
      TWO } StopBitsType;

   typedef enum {
      NONE,
      EVEN,
      ODD } ParityType;

   // Default Constructor
   CSerialPort();

   //////////////////////////////////////////////////////////////////////////
   // Non-default Constructor parameters:
   //   PortNumber:  TTY port number
   //   NumDataBits: Number of data bits (valid range = 5 to 8)
   //   StopBits:    Number of stop bits (1 or 2)
   //   Parity:      Parity type (0=None, 1=Even, 2=Odd)
   //   BaudRate:    BaudRate for communication
   //                Valid Baud Rates:
   //                   0, 50, 75, 110, 134, 150, 200, 300, 600,
   //                   1200, 1800, 2400, 4800, 9600, 19200, 38400
   //////////////////////////////////////////////////////////////////////////
   CSerialPort(const char* Port, int NumDataBits,
               StopBitsType StopBits, ParityType Parity, int BaudRate);

   ~CSerialPort();

   bool Open(const char* Port,
             int NumDataBits, StopBitsType StopBits,
             ParityType Parity, int BaudRate);

   void Close();

   const char* GetPort() const { return mPort.c_str(); }

   bool IsConnected() const { return mPortInitialized; }

   int  Write(char *DataBuffer, int SizeInBytes);
   int  Read(char *DataBuffer, int MaxSizeToRead);

private:

   HANDLE      mHandle;
   std::string mPort;
   bool        mPortInitialized;

}; 

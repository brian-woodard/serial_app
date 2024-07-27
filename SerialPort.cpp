
#include "SerialPort.h"

CSerialPort::CSerialPort()
{
   mHandle = nullptr;
   mPortInitialized = false;  // Port not yet initialized
}

CSerialPort::CSerialPort(const char* Port, 
                         int NumDataBits, StopBitsType StopBits,
                         ParityType Parity, int BaudRate)
{
   Open(Port, NumDataBits, StopBits, Parity, BaudRate);
}

CSerialPort::~CSerialPort()
{
   if (mPortInitialized)
      Close();
}

void CSerialPort::Close()
{
   // Set port to no longer initialized
   mPortInitialized = false;
   // Close the handle
   CloseHandle(mHandle);
}

bool CSerialPort::Open(const char* Port,
                       int NumDataBits, StopBitsType StopBits,
                       ParityType Parity, int BaudRate)
{
   std::string device;

   mPortInitialized = false;  // Port not yet initialized
   mPort = Port;

   /*
    * Munge the string supplied by the user into a Windows filename.
    *
    * Windows supports opening a few "legacy" devices (including
    * COM1-9) by specifying their names verbatim as a filename to
    * open. (Thus, no files can ever have these names. See
    * <http://msdn2.microsoft.com/en-us/library/aa365247.aspx>
    * ("Naming a File") for the complete list of reserved names.)
    *
    * However, this doesn't let you get at devices COM10 and above.
    * For that, you need to specify a filename like "\\.\COM10".
    * This is also necessary for special serial and serial-like
    * devices such as \\.\WCEUSBSH001. It also works for the "legacy"
    * names, so you can do \\.\COM1 (verified as far back as Win95).
    * See <http://msdn2.microsoft.com/en-us/library/aa363858.aspx>
    * (CreateFile() docs).
    *
    * So, we believe that prepending "\\.\" should always be the
    * Right Thing. However, just in case someone finds something to
    * talk to that doesn't exist under there, if the serial line
    * contains a backslash, we use it verbatim. (This also lets
    * existing configurations using \\.\ continue working.)
    */

   device = "\\\\.\\";
   device = Port;

   mHandle = CreateFileA(device.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (mHandle == INVALID_HANDLE_VALUE)
   {
      return false;
   }

   // Flush away any bytes previously read or written.
   BOOL success = FlushFileBuffers(mHandle);
   if (!success)
   {
      CloseHandle(mHandle);
      return false;
   }

   // Configure read and write operations to time out after 100 ms.
   COMMTIMEOUTS timeouts = { 0 };
   timeouts.ReadIntervalTimeout = 0;
   timeouts.ReadTotalTimeoutConstant = 100;
   timeouts.ReadTotalTimeoutMultiplier = 0;
   timeouts.WriteTotalTimeoutConstant = 100;
   timeouts.WriteTotalTimeoutMultiplier = 0;

   success = SetCommTimeouts(mHandle, &timeouts);
   if (!success)
   {
      CloseHandle(mHandle);
   }

   // Set the baud rate and other options.
   DCB state = { 0 };
   state.DCBlength = sizeof(DCB);
   state.BaudRate = BaudRate;
   state.ByteSize = (BYTE)NumDataBits;

   switch (Parity)
   {
      case EVEN:
         state.Parity = EVENPARITY;
         break;
      case ODD:
         state.Parity = ODDPARITY;
         break;
      default:
         state.Parity = NOPARITY;
   }

   switch (StopBits)
   {
      case TWO:
         state.StopBits = TWOSTOPBITS;
         break;
      default:
         state.StopBits = ONESTOPBIT;
   }

   success = SetCommState(mHandle, &state);
   if (!success)
   {
      CloseHandle(mHandle);
      return false;
   }

   mPortInitialized = true;

   return true;
}

int CSerialPort::Write(char *DataBuffer, int SizeInBytes)
{
   DWORD num_bytes_written;

   if (!mPortInitialized)
      return -1;

   BOOL success = WriteFile(mHandle, DataBuffer, SizeInBytes, &num_bytes_written, NULL);
   if (!success)
   {
      return -1;
   }
   if (num_bytes_written != (DWORD)SizeInBytes)
   {
      return -1;
   }

   return num_bytes_written;
}

int CSerialPort::Read(char *DataBuffer, int MaxSizeToRead)
{
   DWORD num_bytes_read;

   if (!mPortInitialized)
      return -1;

   BOOL success = ReadFile(mHandle, DataBuffer, MaxSizeToRead, &num_bytes_read, NULL);
   if (!success)
   {
      return -1;
   }
   return num_bytes_read;
}

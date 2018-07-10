/*******************************************************************************
 RTL_I2C.h

 High-level functions that wrap the Arduino I2C TwoWire interface
 R. Terry Lessly, July 2017
 Based on I2C_Anything by Nick Gammon, 2012
*******************************************************************************/
#ifndef _RTL_I2C_h_
#define _RTL_I2C_h_

#include <Arduino.h>
#include <Wire.h>
#include <Streaming.h>


//******************************************************************************
/// Summary: Reads a series of bytes from the I2C network.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: pDest - A pointer to a buffer to write the bytes that were read
/// Param: sizeDest - The size of the destination buffer, in bytes
///
/// Returns: The actual number of bytes read. This value may be less than or
///          equal to the number of bytes requested in the sizeDest parameter.
//******************************************************************************
uint16_t inline I2C_Read(TwoWire& twi, uint8_t* pDest, size_t sizeDest)
{
    uint16_t bytesRead = 0;

    while (bytesRead < sizeDest && twi.available()) pDest[bytesRead++] = twi.read();

    return bytesRead;
}

// Same as above but uses the default I2C port (aka Wire).
uint16_t inline I2C_Read(uint8_t* pDest, size_t sizeDest) { return I2C_Read(Wire, pDest, sizeDest); }


//******************************************************************************
/// Summary: Reads a typed value from the I2C network.
///
/// Template param: T - The data type to read
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: value - A reference to a variable of type <T> to store the data value
///                read from the I2C network.
///
/// Returns: The actual number of bytes read. This value may be less than or
///          equal to sizeof(T).
///
/// Remarks:
/// This function can be used to read any data type from the I2C network, from a
/// single byte to complex structs. The length of the read operation is determined
/// by sizeof(T). If the return value is less than sizeof(T) then the read operation
/// failed and the value of the value parameter should not be considered valid.
///
/// NOTE: This operation assumes that both the sender and receiver use the same
///       byte order (big-endian or little-endian).
//******************************************************************************
template <typename T>
uint16_t inline I2C_Read(TwoWire& twi, T& value)
{
    return I2C_Read(twi, (uint8_t*)&value, sizeof value);
}

// Same as above but uses the default I2C port (aka Wire).
template <typename T>
uint16_t inline I2C_Read(T& value) { return I2C_Read(Wire, value); }


//******************************************************************************
/// Summary: Writes a typed value to the I2C network.
///
/// Template param: T - The data type to write
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: value - A reference to a variable of type T to write to the I2C network.
///
/// Returns: The actual number of bytes written. This value should always be equal to sizeof(T).
///
/// Remarks:
/// This function can be used to write any data type to the I2C network, from a
/// single byte to complex structs. The length of the write operation  is determined
/// by sizeof(T).
///
/// NOTE: This operation assumes that both the sender and receiver use the same
///       byte order (big-endian or little-endian).
//******************************************************************************
template <typename T>
uint16_t inline I2C_Write(TwoWire& twi, const T& value)
{
    twi.write((uint8_t*) &value, sizeof(value));

    return sizeof(value);
}

// Same as above but uses the default I2C port (aka Wire).
template <typename T>
uint16_t inline I2C_Write(const T& value) { return I2C_Write(Wire, value); }


//******************************************************************************
/// Summary: Sends a complete message over the I2C network and optionally receives a response.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: pMessage - A pointer to a buffer containing the message to send.
/// Param: messageLength - The length of the message.
/// Param: pResponse - A pointer to a buffer to hold the response.
/// Param: responseLength - The expected length of the response.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message and
/// receiving a response. If no response is received after a certain amount of
/// time the function times-out and returns error code 4. The actual response
/// returned may be shorter than the responseLength parameter, but it will never
/// be longer. Any response bytes recieved in excess of responseLength are ignored.
//******************************************************************************
uint8_t inline I2C_SendMessage(TwoWire& twi, const uint8_t address, uint8_t const* pMessage, const uint8_t messageLength, uint8_t* pResponse, const uint8_t responseLength)
{
    twi.beginTransmission(address);
    twi.write(pMessage, messageLength);

    auto isRequest = (pResponse != NULL);
    auto statusCode = twi.endTransmission(!isRequest);

    if (statusCode == 0 && isRequest)
    {
        auto bytesRead = twi.requestFrom(address, responseLength);

        for (auto i = 0; i < bytesRead; i++)
            if (i < responseLength) pResponse[i] = (uint8_t)twi.read();

        statusCode = (bytesRead > 0) ? 0 : 4;
    }

    return statusCode;
}

// Same as above but uses the default I2C port (aka Wire).
uint8_t inline I2C_SendMessage(const uint8_t address, uint8_t const* pMessage, const uint8_t messageLength, uint8_t* pResponse, const uint8_t responseLength)
{
    return I2C_SendMessage(Wire, address, pMessage, messageLength, pResponse, responseLength);
}


//******************************************************************************
/// Summary: Sends a complete message over the I2C network. No response is expected
///          for the message.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: pMessage - A pointer to a buffer containing the message to send.
/// Param: messageLength - The length of the message.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message over the
/// I2C network. This method does not expect a response from the receiving device.
//******************************************************************************
uint8_t inline I2C_SendMessage(TwoWire& twi, const uint8_t address, uint8_t const* pMessage, const uint8_t messageLength)
{
    twi.beginTransmission(address);
    twi.write(pMessage, messageLength);

    auto statusCode = twi.endTransmission(true);

    return statusCode;
}

// Same as above but uses the default I2C port (aka Wire).
uint8_t inline I2C_SendMessage(const uint8_t address, uint8_t const* pMessage, const uint8_t messageLength) { return I2C_SendMessage(Wire, address, pMessage, messageLength); }


//******************************************************************************
/// Summary: Sends a complete message over the I2C network. The message is contained
///           in a typed object. No response is expected for the message.
///
/// Template param: T - The data type of the message to send. This will usually be
///                     a struct containing the message data.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: message - A reference to a variable of type T containing the message
///                  to send. Messages are typically defined as structs.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message over the
/// I2C network. This method does not expect a response from the receiving device.
///
/// Messages are typically defined as structs containing one or more values to send.
/// To ensure reliable receipt of the message, the struct should NOT contain and
/// virtual methods, or subclass a struct that contains virtual methods (virtual
/// methods insert hidden fields into the struct).
///
/// NOTE: In order for structs to be populated correctly, this operation assumes that
///       both the sender and receiver use the same byte order (big-endian or little-endian).
//******************************************************************************
template <typename T>
uint8_t inline I2C_SendMessage(TwoWire& twi, const uint8_t address, const T& message)
{
    twi.beginTransmission(address);
    twi.write((uint8_t *) &message, sizeof(message));

    uint8_t statusCode = twi.endTransmission(true);

    return statusCode;
}

// Same as above but uses the default I2C port (aka Wire).
template <typename T>
uint8_t inline I2C_SendMessage(const uint8_t address, const T& message) { return I2C_SendMessage(Wire, address, message); }


//******************************************************************************
/// Summary: Sends a command over the I2C network and receives a response.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: cmd - A 1-byte command code.
/// Param: pCmdData - A pointer to data to be sent along with the command.
/// Param: pCmdDataLength - the length of the command data.
/// Param: pResponse - A pointer to a buffer to hold the response.
/// Param: responseLength - The expected length of the response.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message and
/// receiving a response over the I2C network. If no response is received after
/// a certain amount of time the function times-out and returns error code 4. The
/// actual response returned may be shorter than the responseLength parameter,
/// but it will never be longer. Any response bytes recieved in excess of
/// responseLength are ignored.
///
/// If the command does not expect a response the pResponse pointer and responseLength
/// arguments can be omitted in the function call.
//******************************************************************************
uint8_t inline I2C_SendCommand(TwoWire& twi, const uint8_t address, const uint8_t cmd, uint8_t const* pCmdData, size_t cmdLength, uint8_t* pResponse=NULL, size_t responseLength=0)
{
    twi.beginTransmission(address);
    twi.write(cmd);
    twi.write(pCmdData, (pCmdData == NULL) ? 0 : cmdLength);

    auto isRequest = (pResponse != NULL);
    auto statusCode = twi.endTransmission(!isRequest);

    if (statusCode == 0 && isRequest)
    {
        auto bytesRead = twi.requestFrom(address, responseLength);

        for (auto i = 0; i < bytesRead; i++)
            if (i < responseLength) pResponse[i] = (uint8_t)twi.read();

        statusCode = (bytesRead > 0) ? 0 : 4;
    }

    return statusCode;
}

// Same as above but uses the default I2C port (aka Wire).
uint8_t inline I2C_SendCommand(const uint8_t address, const uint8_t cmd, uint8_t const* pCmdData, size_t cmdLength, uint8_t* pResponse=NULL, size_t responseLength=0)
{
    return I2C_SendCommand(Wire, address, cmd, pCmdData, cmdLength, pResponse, responseLength);
}


//******************************************************************************
/// Summary: Sends a command over the I2C network. The command requires no additional
///          data and no response is expected.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: cmd - A 1-byte command code.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
//******************************************************************************
uint8_t inline I2C_SendCommand(TwoWire& twi, const uint8_t address, const uint8_t cmd)
{
    return I2C_SendCommand(twi, address, cmd, NULL, 0, NULL, 0);
}

// Same as above but uses the default I2C port (aka Wire).
uint8_t inline I2C_SendCommand(const uint8_t address, const uint8_t cmd) { return I2C_SendCommand(Wire, address, cmd); }



//******************************************************************************
/// Summary: Sends a command with data over the I2C network. No response is expected.
///
/// Template param: T - The data type of the message to send. This will usually be
///                     a struct containing the message data.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: cmd - A 1-byte command code.
/// Param: pResponse - A pointer to a buffer to hold the response.
/// Param: responseLength - The expected length of the response.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
//******************************************************************************
template <typename T>
uint8_t inline I2C_SendCommand(TwoWire& twi, const uint8_t address, const uint8_t cmd, const T& cmdData)
{
    return I2C_SendCommand(twi, address, cmd, (uint8_t*)cmdData, sizeof(cmdData), NULL, 0);
}

// Same as above but uses the default I2C port (aka Wire).
template <typename T>
uint8_t inline I2C_SendCommand(const uint8_t address, const uint8_t cmd, const T& cmdData) { return I2C_SendCommand(Wire, address, cmd, cmdData); }



//******************************************************************************
/// Summary: Sends a command over the I2C network and receive the response. The
///          command does not send any data.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: cmd - A 1-byte command code.
/// Param: cmdData - The data to send with the command. This will often be an instance
///        of a struct containing the command data.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message and
/// receiving a response over the I2C network. If no response is received after
/// a certain amount of time the function times-out and returns error code 4. The
/// actual response returned may be shorter than the responseLength parameter,
/// but it will never be longer. Any response bytes recieved in excess of
/// responseLength are ignored.
//******************************************************************************
uint8_t inline I2C_SendRequest(TwoWire& twi, const uint8_t address, const uint8_t cmd, uint8_t* pResponse, size_t responseLength)
{
    return I2C_SendCommand(twi, address, cmd, NULL, 0, pResponse, responseLength);
}

// Same as above but uses the default I2C port (aka Wire).
uint8_t inline I2C_SendRequest(const uint8_t address, const uint8_t cmd, uint8_t* pResponse, size_t responseLength) 
{ 
    return I2C_SendRequest(Wire, address, cmd, pResponse, responseLength); 
}


//******************************************************************************
/// Summary: Sends a command over the I2C network and receive the response. The
///          command does not send any data. The response is stored in a typed object.
///
/// Template param: R - The data type of the response data. This will usually be
///                     a struct containing the response data.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: cmd - A 1-byte command code.
/// Param: responseData - A reference to an object to hold the response. This will often be an instance
///        of a struct containing the command data.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message and
/// receiving a response over the I2C network. If no response is received after
/// a certain amount of time the function times-out and returns error code 4. The
/// actual response returned may be shorter than the responseLength parameter,
/// but it will never be longer. Any response bytes recieved in excess of
/// responseLength are ignored.
//******************************************************************************
template <typename R>
uint8_t inline I2C_SendRequest(TwoWire& twi, const uint8_t address, const uint8_t cmd, R& responseData)
{
    return I2C_SendCommand(twi, address, cmd, NULL, 0, (uint8_t*)&responseData, sizeof(responseData));
}

// Same as above but uses the default I2C port (aka Wire).
template <typename R>
uint8_t inline I2C_SendRequest(const uint8_t address, const uint8_t cmd, R& responseData) { return I2C_SendRequest(Wire, address, cmd, responseData); }



//******************************************************************************
/// Summary: Sends a command over the I2C network and receive the response. The
///          command data and response data are stored in typed objects.
///
/// Template param: T - The data type of the command data. This will usually be
///                     a struct containing the command data.
/// Template param: R - The data type of the response data. This will usually be
///                     a struct containing the response data.
///
/// Param: twi - A reference to a TwoWire object representing the I2C port to use.
/// Param: address - The I2C address of the target device.
/// Param: cmd - A 1-byte command code.
/// Param: cmdData - The data to send with the command. This will often be an instance
///        of a struct containing the command data.
/// Param: responseData - A reference to an object to hold the response. This will often be an instance
///        of a struct containing the command data.
///
/// Returns: A status code indicating the result of the send operation. A return
///          code of 0 (zero) indicates a successful send. A non-zero return code
///          indicates a failure. See the Arduino reference for the TwoWire::endTransmission()
///          function for a full description of I2C status codes.
///
/// Remarks:
/// This function executes the complete I2C protocol for sending a message and
/// receiving a response over the I2C network. If no response is received after
/// a certain amount of time the function times-out and returns error code 4. The
/// actual response returned may be shorter than the responseLength parameter,
/// but it will never be longer. Any response bytes recieved in excess of
/// responseLength are ignored.
//******************************************************************************
template <typename T, typename R>
uint8_t inline I2C_SendRequest(TwoWire& twi, const uint8_t address, const uint8_t cmd, const T& cmdData, R& responseData)
{
    return I2C_SendCommand(twi, address, cmd, (uint8_t*)&cmdData, sizeof(cmdData), (uint8_t*)&responseData, sizeof(responseData));
}

// Same as above but uses the default I2C port (aka Wire).
template <typename T, typename R>
uint8_t inline I2C_SendRequest(const uint8_t address, const uint8_t cmd, const T& cmdData, R& responseData) { I2C_SendRequest(Wire, address, cmd, cmdData, responseData); }

#endif


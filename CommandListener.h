/*******************************************************************************
 Header file for the CommandListener class.
*******************************************************************************/
#ifndef _CommandListener_h_
#define _CommandListener_h_

#include <RTL_StdLib.h>
#include <RTL_I2C.h>
#include <RTL_EventFramework.h>
#include "CommandProtocol.h"


class CommandListener : public EventSource, public IEventListener
{
    DECLARE_CLASSNAME;

    //public:  static const EVENT_ID ResponseEvent = (EventSourceID::Task | EventCode::Response);

    //private: static const int DEFERRED_RESPONSE_LIST_SIZE = 5;

    public: byte CommandBuffer[32];

    public: byte ResponseBuffer[32];

    public: bool CommandPending;

    public: bool ResponsePending;


    /***************************************************************************
    Constructors / Destructors
    ***************************************************************************/
    protected: CommandListener(byte deviceID=0) : _myDeviceID(deviceID) 
    {
        _id = "CommandListener";
    };

    /***************************************************************************
    Public implementation
    ***************************************************************************/
    public: void Poll();
    public: void Begin();
    public: void OnEvent(const Event* pEvent);
    // public: void OnCommandReceived(const CommandMessage* pCommand);
    // public: void OnCommandReceived(TwoWire& twi, int messageLength);
    public: const CommandResponse* GetResponse();

    public: virtual void SendResponse(TwoWire& twi);

    /***************************************************************************
    Shared implementation
    ***************************************************************************/
    protected: virtual void OnBegin() { };
    protected: virtual void OnCommand(const CommandMessage* pCommand);
    protected: virtual bool IsResponseExpected(const CommandMessage* pCommand) { return false; };
    
    protected: void DefaultCommandHandler(const CommandMessage* pCommand);
    protected: void PostResponse(CommandResponse* pResponse);
    //protected: void PostDeferredResponse(CommandResponse* pResponse);

    /***************************************************************************
    Internal implementation
    ***************************************************************************/
    //private: void HandleCommand(const CommandMessage* pCommand);
    //private: void HandleQueryResponseReady(const CommandQueryResponseReady& command);

    /***************************************************************************
    Internal state
    ***************************************************************************/
    private: byte _myDeviceID;

    // private: struct ResponseItem 
    // {
        // bool InUse;
        // CommandResponse* pResponse;
      
        // ResponseItem() : InUse(false), pResponse(NULL) { };
    // };

    //private: ResponseItem _deferredResponseList[DEFERRED_RESPONSE_LIST_SIZE];
};

#endif


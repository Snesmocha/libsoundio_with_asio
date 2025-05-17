#ifndef CASIO_H
#define CASIO_H

#define COBJMACROS // for COM-style access (lpVtbl)
#include <windows.h>

#include <stdint.h>

// asioMessage selectors
typedef enum
{
	kAsioSelectorSupported = 1,	// selector in <value>, returns 1L if supported,
								// 0 otherwise
    kAsioEngineVersion,			// returns engine (host) asio implementation version,
								// 2 or higher
	kAsioResetRequest,			// request driver reset. if accepted, this
								// will close the driver (ASIO_Exit() ) and
								// re-open it again (ASIO_Init() etc). some
								// drivers need to reconfigure for instance
								// when the sample rate changes, or some basic
								// changes have been made in ASIO_ControlPanel().
								// returns 1L; note the request is merely passed
								// to the application, there is no way to determine
								// if it gets accepted at this time (but it usually
								// will be).
	kAsioBufferSizeChange,		// not yet supported, will currently always return 0L.
								// for now, use kAsioResetRequest instead.
								// once implemented, the new buffer size is expected
								// in <value>, and on success returns 1L
	kAsioResyncRequest,			// the driver went out of sync, such that
								// the timestamp is no longer valid. this
								// is a request to re-start the engine and
								// slave devices (sequencer). returns 1 for ok,
								// 0 if not supported.
	kAsioLatenciesChanged, 		// the drivers latencies have changed. The engine
								// will refetch the latencies.
	kAsioSupportsTimeInfo,		// if host returns true here, it will expect the
								// callback bufferSwitchTimeInfo to be called instead
								// of bufferSwitch
	kAsioSupportsTimeCode,		// 
	kAsioMMCCommand,			// unused - value: number of commands, message points to mmc commands
	kAsioSupportsInputMonitor,	// kAsioSupportsXXX return 1 if host supports this
	kAsioSupportsInputGain,     // unused and undefined
	kAsioSupportsInputMeter,    // unused and undefined
	kAsioSupportsOutputGain,    // unused and undefined
	kAsioSupportsOutputMeter,   // unused and undefined
	kAsioOverload,              // driver detected an overload

	kAsioNumMessageSelectors
} ASIO_MESSAGE_SELECTOR;



typedef enum
{
	kAsioEnableTimeCodeRead = 1,	// no arguments
	kAsioDisableTimeCodeRead,		// no arguments
	kAsioSetInputMonitor,			// ASIOInputMonitor* in params
	kAsioTransport,					// ASIOTransportParameters* in params
	kAsioSetInputGain,				// ASIOChannelControls* in params, apply gain
	kAsioGetInputMeter,				// ASIOChannelControls* in params, fill meter
	kAsioSetOutputGain,				// ASIOChannelControls* in params, apply gain
	kAsioGetOutputMeter,			// ASIOChannelControls* in params, fill meter
	kAsioCanInputMonitor,			// no arguments for kAsioCanXXX selectors
	kAsioCanTimeInfo,
	kAsioCanTimeCode,
	kAsioCanTransport,
	kAsioCanInputGain,
	kAsioCanInputMeter,
	kAsioCanOutputGain,
	kAsioCanOutputMeter,
	kAsioOptionalOne,
	
	//	DSD support
	//	The following extensions are required to allow switching
	//	and control of the DSD subsystem.
	kAsioSetIoFormat			= 0x23111961,		/* ASIOIoFormat * in params.			*/
	kAsioGetIoFormat			= 0x23111983,		/* ASIOIoFormat * in params.			*/
	kAsioCanDoIoFormat			= 0x23112004,		/* ASIOIoFormat * in params.			*/
	
	// Extension for drop out detection
	kAsioCanReportOverload			= 0x24042012,	/* return ASE_SUCCESS if driver can detect and report overloads */
	
	kAsioGetInternalBufferSamples	= 0x25042012	/* ASIOInternalBufferInfo * in params. Deliver size of driver internal buffering, return ASE_SUCCESS if supported */
} ASIO_SET_GET;

enum ASIOIoFormatType_e
{
	kASIOFormatInvalid = -1,
	kASIOPCMFormat = 0,
	kASIODSDFormat = 1,
};


// asio sdk background calls reimplemented cause god is dead and fuck c++
// fundimentally all this is here for is to hook into an asio driver
typedef double ASIOSampleRate;
typedef int64_t ASIOSamples;
typedef int64_t ASIOTimeStamp;
typedef int32_t ASIOBool;
typedef int32_t ASIOError;

typedef struct ASIOClockSource 
{
    int32_t index;
    int32_t associatedChannel;
    int32_t associatedGroup;
    ASIOBool isCurrentSource;
    char name[32];
} ASIOClockSource;

typedef struct ASIOChannelInfo 
{
    int32_t channel;
    ASIOBool isInput;
    ASIOBool isActive;
    int32_t channelGroup;
    char name[32];
    int32_t type; // ASIOSampleType
} ASIOChannelInfo;

typedef struct ASIOBufferInfo 
{
    ASIOBool isInput;
    int32_t channelNum;
    void *buffers[2]; // double buffer
} ASIOBufferInfo;


typedef struct ASIOTimeCode
{
	double          speed;                  // speed relation (fraction of nominal speed)
	                                        // optional; set to 0. or 1. if not supported
	ASIOSamples     timeCodeSamples;        // time in samples
	uint32_t  flags;                  // some information flags (see below)
	char future[64];			// FUCK YOU STEINBURG
} ASIOTimeCode;

typedef struct ASIOTimeInfo
{
	double          speed;                  // absolute speed (1. = nominal)
	ASIOTimeStamp   systemTime;             // system time related to samplePosition, in nanoseconds
	                                        // on mac, must be derived from Microseconds() (not UpTime()!)
	                                        // on windows, must be derived from timeGetTime()
	ASIOSamples     samplePosition;
	ASIOSampleRate  sampleRate;             // current rate
	uint32_t flags;                    // (see below)
	char reserved[12];
} ASIOTimeInfo;

typedef struct ASIOTime
{
	uint32_t reserved[4];
	ASIOTimeInfo time_info;
	ASIOTimeCode time_code;
} ASIOTime; // Forward declaration

typedef struct ASIOCallbacks {
    void (*bufferSwitch)(int32_t doubleBufferIndex, ASIOBool directProcess);
    void (*sampleRateDidChange)(ASIOSampleRate sRate);
    long (*asioMessage)(int32_t selector, int32_t value, void *message, double *opt);
    ASIOTime* (*bufferSwitchTimeInfo)(ASIOTime *params, int32_t doubleBufferIndex, ASIOBool directProcess);
} ASIOCallbacks;

typedef struct ASIOInternalBufferInfo
{
	int32_t inputSamples;			// size of driver's internal input buffering which is included in getLatencies
	int32_t outputSamples;			// size of driver's internal output buffering which is included in getLatencies
} ASIOInternalBufferInfo;

// ---- Interface Forward Declaration ----
typedef struct IASIOVtbl IASIOVtbl;

typedef struct IASIO 
{
    const IASIOVtbl* lpVtbl;
} IASIO;

struct IASIOVtbl 
{
    HRESULT (__stdcall* QueryInterface)(IASIO*, REFIID, void**);
    ULONG (__stdcall* AddRef)(IASIO*);
    ULONG (__stdcall* Release)(IASIO*);
	
    ASIOBool (__stdcall* init)(IASIO*, void* sysHandle);
    void (__stdcall* getDriverName)(IASIO*, char* name);
    int32_t (__stdcall* getDriverVersion)(IASIO*);
    void (__stdcall* getErrorMessage)(IASIO*, char* string);

    ASIOError (__stdcall* start)(IASIO*);
    ASIOError (__stdcall* stop)(IASIO*);
    ASIOError (__stdcall* getChannels)(IASIO*, int32_t *numInputChannels, int32_t *numOutputChannels);
    ASIOError (__stdcall* getLatencies)(IASIO*, int32_t *inputLatency, int32_t *outputLatency);
    ASIOError (__stdcall* getBufferSize)(IASIO*, int32_t *minSize, int32_t *maxSize, int32_t *preferredSize, int32_t *granularity);
    ASIOError (__stdcall* canSampleRate)(IASIO*, ASIOSampleRate sampleRate);
    ASIOError (__stdcall* getSampleRate)(IASIO*, ASIOSampleRate *sampleRate);
    ASIOError (__stdcall* setSampleRate)(IASIO*, ASIOSampleRate sampleRate);
    ASIOError (__stdcall* getClockSources)(IASIO*, ASIOClockSource *clocks, int32_t *numSources);
    ASIOError (__stdcall* setClockSource)(IASIO*, int32_t reference);
    ASIOError (__stdcall* getSamplePosition)(IASIO*, ASIOSamples *sPos, ASIOTimeStamp *tStamp);
    ASIOError (__stdcall* getChannelInfo)(IASIO*, ASIOChannelInfo *info);
    ASIOError (__stdcall* createBuffers)(IASIO*, ASIOBufferInfo *bufferInfos, int32_t numChannels, int32_t bufferSize, ASIOCallbacks *callbacks);
    ASIOError (__stdcall* disposeBuffers)(IASIO*);
    ASIOError (__stdcall* controlPanel)(IASIO*);
    ASIOError (__stdcall* future)(IASIO*, int32_t selector, int32_t *opt);
    ASIOError (__stdcall* outputReady)(IASIO*);
};

typedef IASIO* (*ASIOCreateInstanceFn)(void);

#endif

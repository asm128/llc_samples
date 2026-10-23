#include "llc_runtime.h"
#include "llc_comm.h"
#include "llc_event.h"
#include "llc_array_obj.h"

sttc ::llc::err_t comm_sample_entry_point(::llc::SRuntimeValues & runtimeValues);
LLC_OS_ENTRY_POINT(::comm_sample_entry_point);

stxp ::llc::u1_t CARRIER_FRAME_SIZE	= 32;
stxp ::llc::u2_t MESSAGE_SIZE_LIMIT	= 512;

sttc ::llc::err_t carrierRoundTrip
	( cnst ::llc::SPacket	& packet
	, ::llc::u1_t		frameSize
	, ::llc::SCommComm	& commandReceived
	, ::llc::au0_t		& payloadReceived
	, ::llc::u2_t		& frameCount
	) {
	::llc::SCommPacketizer	packetizer	= {};
	::llc::aobj<::llc::au0_t>	carrier		= {};
	::llc::u0_t		frameStorage[CARRIER_FRAME_SIZE]	= {};
	::llc::vu0_t		frameBuffer	= {frameStorage, ::llc::min(frameSize, CARRIER_FRAME_SIZE)};
	llc_necs(::llc::commPacketizerBegin(packetizer, packet));
	while(false == packetizer.Complete) {
		::llc::u2_t		sizeFrame	= {};
		llc_necs(::llc::commPacketNext(packetizer, frameBuffer, sizeFrame));
		if_zero_fef(sizeFrame, "%s", "The packetizer produced an empty carrier frame.");
		::llc::au0_t		frame		= {};
		llc_necs(frame.append(frameStorage, sizeFrame));
		llc_necs(carrier.push_back(frame));
	}

	::llc::u0_t		messageStorage[MESSAGE_SIZE_LIMIT]	= {};
	::llc::SCommReassembly	assembly	= {};
	llc_necs(::llc::commReassemblyReset(assembly, {messageStorage, ::llc::size(messageStorage)}));
	payloadReceived.clear();
	bool received	= false;
	for(::llc::u2_t iFrame = 0; iFrame < carrier.size(); ++iFrame) {
		::llc::vcu0_t		bytesFrame	= carrier[iFrame];
		::llc::SPacket		packetFrame	= {};
		llc_necs(::llc::commPacketLoad(bytesFrame, packetFrame));
		::llc::vcu0_t		message		= {};
		bool			complete	= false;
		llc_necs(::llc::commPacketReceive(assembly, packetFrame, message, complete));
		if(complete) {
			commandReceived	= packetFrame.Comm;
			payloadReceived	= message;
			received	= true;
		}
	}
	if_true_fef(false == received, "%s", "The carrier queue ended before a message was complete.");
	frameCount	= carrier.size();
	rtrn 0;
}

sttc ::llc::err_t verifyEvent(::llc::vcu0_t serialized, cnst ::llc::SEventCommand & expected) {
	::llc::SEventCommand	received	= {};
	llc_necs(received.Load(serialized));
	if_true_fef(serialized.size(), "Unexpected trailing event bytes: %" LLC_FMT_U2 ".", serialized.size());
	if_true_fef(received.Type != expected.Type, "Unexpected event type: %i. Expected: %i.", (int)received.Type, (int)expected.Type);
	::llc::vcu0_t		dataReceived	= received.Data;
	::llc::vcu0_t		dataExpected	= expected.Data;
	if_true_fef(dataReceived != dataExpected, "Unexpected event data size: %" LLC_FMT_U2 ". Expected: %" LLC_FMT_U2 ".", dataReceived.size(), dataExpected.size());
	rtrn 0;
}

sttc ::llc::err_t comm_sample_entry_point(::llc::SRuntimeValues & runtimeValues) {
	(void)runtimeValues;
	::llc::SCommCoordinator	coordinator	= {};
	::llc::SCommEndpoint	sender		= {};
	::llc::SCommEndpoint	receiver	= {};
	llc_necs(::llc::commCoordinatorInit(coordinator, 1, CARRIER_FRAME_SIZE));
	llc_necs(::llc::commCoordinatorJoin(coordinator, CARRIER_FRAME_SIZE, sender));
	llc_necs(::llc::commCoordinatorJoin(coordinator, CARRIER_FRAME_SIZE, receiver));

	stxp ::llc::u0_t dataFirst []	= {'o', 'n', 'e'};
	stxp ::llc::u0_t dataSecond[]	= {'t', 'w', 'o'};
	::llc::SEventCommand eventsSmall[] =
		{ {::llc::COMMAND_Ok  , {dataFirst , ::llc::size(dataFirst )}}
		, {::llc::COMMAND_Busy, {dataSecond, ::llc::size(dataSecond)}}
		};
	::llc::au0_t serializedSmall[::llc::size(eventsSmall)]	= {};
	::llc::vcu0_t payloadsSmall [::llc::size(eventsSmall)]	= {};
	for(::llc::u2_t iEvent = 0; iEvent < ::llc::size(eventsSmall); ++iEvent) {
		llc_necs(eventsSmall[iEvent].Save(serializedSmall[iEvent]));
		payloadsSmall[iEvent]	= serializedSmall[iEvent];
	}
	::llc::au0_t payloadPacked	= {};
	llc_necs(::llc::commPayloadPack({payloadsSmall, ::llc::size(payloadsSmall)}, payloadPacked));

	::llc::SPacket packetPacked	= {};
	packetPacked.Comm.Command	= ::llc::COMM_COMMAND_Payload;
	packetPacked.Comm.Type		= ::llc::COMM_COMMAND_TYPE_Request;
	packetPacked.Comm.Packed	= 1;
	packetPacked.Network		= coordinator.Network;
	packetPacked.Source		= sender.Address.Endpoint;
	packetPacked.Target		= receiver.Address.Endpoint;
	llc_necs(::llc::commMessageNext(sender, packetPacked.Message));
	packetPacked.Payload		= payloadPacked;

	::llc::SCommComm commandReceived	= {};
	::llc::au0_t payloadReceived		= {};
	::llc::u2_t frameCountPacked		= {};
	llc_necs(::carrierRoundTrip(packetPacked, sender.FrameSize, commandReceived, payloadReceived, frameCountPacked));
	if_true_fef(0 == commandReceived.Packed, "%s", "The packed carrier flag was lost.");
	::llc::SCommPayloadReader payloadReader	= {};
	llc_necs(::llc::commPayloadLoad(payloadReceived, payloadReader));
	if_true_fef(payloadReader.Count != ::llc::size(eventsSmall), "Unexpected packed event count: %" LLC_FMT_U2 ".", payloadReader.Count);
	for(::llc::u2_t iEvent = 0; iEvent < payloadReader.Count; ++iEvent) {
		::llc::vcu0_t serializedEvent	= {};
		llc_necs(::llc::commPayloadNext(payloadReader, serializedEvent));
		llc_necs(::verifyEvent(serializedEvent, eventsSmall[iEvent]));
	}

	stxp ::llc::sc_t textLarge[] =
		"SEvent remains one carrier-neutral sentence while the lower layer folds it across small frames and restores it before Load.";
	::llc::SEventCommand eventLarge
		= {::llc::COMMAND_Write, {(cnst ::llc::u0_t*)textLarge, ::llc::size(textLarge) - 1}};
	::llc::au0_t serializedLarge	= {};
	llc_necs(eventLarge.Save(serializedLarge));
	::llc::SPacket packetLarge	= {};
	packetLarge.Comm.Command	= ::llc::COMM_COMMAND_Payload;
	packetLarge.Comm.Type		= ::llc::COMM_COMMAND_TYPE_Request;
	packetLarge.Network		= coordinator.Network;
	packetLarge.Source		= sender.Address.Endpoint;
	packetLarge.Target		= receiver.Address.Endpoint;
	llc_necs(::llc::commMessageNext(sender, packetLarge.Message));
	packetLarge.Payload		= serializedLarge;

	::llc::u2_t frameCountLarge	= {};
	llc_necs(::carrierRoundTrip(packetLarge, sender.FrameSize, commandReceived, payloadReceived, frameCountLarge));
	if_true_fef(frameCountLarge < 2, "Expected multipart output. Carrier frames: %" LLC_FMT_U2 ".", frameCountLarge);
	llc_necs(::verifyEvent(payloadReceived, eventLarge));

	info_printf
		( "Communication sample passed. Packed events: %" LLC_FMT_U2 ". Packed frames: %" LLC_FMT_U2 ". Multipart frames: %" LLC_FMT_U2 ". Frame size: %u."
		, (::llc::u2_t)::llc::size(eventsSmall)
		, frameCountPacked
		, frameCountLarge
		, (::llc::u2_t)sender.FrameSize
		);
	rtrn 0;
}

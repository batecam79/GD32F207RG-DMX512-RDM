/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdint>
#include <cstdio>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"
#include "ledblink.h"

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"

#include "e131bridge.h"
#include "e131params.h"
#include "e131reboot.h"
#include "e131msgconst.h"
#include "lightset.h"

#include "pixeltestpattern.h"
#include "pixeltype.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmxmulti.h"
#include "ws28xxdmxstartstop.h"

// RDMNet LLRP Device Only
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"
#include "rdmdeviceparams.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashrom.h"
#include "spiflashstore.h"
#include "storedisplayudf.h"
#include "storee131.h"
#include "storenetwork.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"
#include "storews28xxdmx.h"
// Version control
#include "firmwareversion.h"
#include "software_version.h"
// HTTP/JSON Remote configuration
#if defined (ENABLE_HTTPD)
# include "mdns.h"
# include "mdnsservices.h"
#endif

class Reboot final : public RebootHandler {
public:
	Reboot() {}
	~Reboot() override {}

	void Run() override {
		WS28xxMulti::Get()->Blackout();
	}
};

void main(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	FlashRom flashRom;
	SpiFlashStore spiFlashStore;

	fw.Print("Ethernet sACN E1.31 " "\x1b[32m" "Pixel controller {8x 4 Universes}" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

#if defined (ENABLE_HTTPD)
	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=sACN E1.31 Pixel");
	mDns.Print();
#endif

	display.TextStatus(E131MsgConst::PARAMS, Display7SegmentMessage::INFO_BRIDGE_PARMAMS, CONSOLE_YELLOW);

	E131Bridge bridge;

	StoreE131 storeE131;
	E131Params e131params(&storeE131);

	if (e131params.Load()) {
		e131params.Set(&bridge);
		e131params.Dump();
	}

	PixelDmxConfiguration pixelDmxConfiguration;

	StoreWS28xxDmx storeWS28xxDmx;
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	DEBUG_PUTS("");

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&pixelDmxConfiguration);
		ws28xxparms.Dump();
	}


	WS28xxDmxMulti pixelDmxMulti(pixelDmxConfiguration);
	pixelDmxMulti.SetPixelDmxHandler(new PixelDmxStartStop);

	bridge.SetOutput(&pixelDmxMulti);

	const auto nUniverses = pixelDmxMulti.GetUniverses();
	const auto nActivePorts = pixelDmxMulti.GetOutputPorts();

	uint32_t nPortProtocolIndex = 0;

	for (uint32_t nOutportIndex = 0; nOutportIndex < nActivePorts; nOutportIndex++) {
		auto isSet = false;
		const auto nStartUniversePort = ws28xxparms.GetStartUniversePort(nOutportIndex, isSet);
		for (uint32_t u = 0; u < nUniverses; u++) {
			if (isSet) {
				DEBUG_PRINTF("%u:%u", nPortProtocolIndex, static_cast<uint16_t>(nStartUniversePort + u));
				bridge.SetUniverse(nPortProtocolIndex, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nStartUniversePort + u));
			}
			nPortProtocolIndex++;
		}
	}

	uint8_t nTestPattern;
	PixelTestPattern *pPixelTestPattern = nullptr;

	if ((nTestPattern = ws28xxparms.GetTestPattern()) != 0) {
		pPixelTestPattern = new PixelTestPattern(static_cast<pixelpatterns::Pattern>(nTestPattern), nActivePorts);
		bridge.SetOutput(nullptr);
		hw.SetRebootHandler(new Reboot);

	} else {
		hw.SetRebootHandler(new E131Reboot);
	}

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "sACN Pixel %d-%s:%d", nActivePorts, PixelType::GetType(WS28xxMulti::Get()->GetType()), WS28xxMulti::Get()->GetCount());

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, "GigaDevice Pixel");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.SetRDMFactoryDefaults(new FactoryDefaults);
	llrpOnlyDevice.Init();

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&llrpOnlyDevice);
		rdmDeviceParams.Dump();
	}

	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);
	llrpOnlyDevice.Print();

	bridge.Print();
	pixelDmxMulti.Print();

	display.SetTitle("sACN Pixel 8:%dx%d", nActivePorts, WS28xxMulti::Get()->GetCount());
	display.Set(2, displayudf::Labels::HOSTNAME);
	display.Set(3, displayudf::Labels::IP);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::UNIVERSE);
	display.Set(6, displayudf::Labels::BOARDNAME);
	display.Printf(7, "%d-%s:%d G%d", nActivePorts, PixelType::GetType(pixelDmxConfiguration.GetType()), pixelDmxConfiguration.GetCount(), pixelDmxConfiguration.GetGroupingCount());

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if(displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&bridge);

	RemoteConfig remoteConfig(remoteconfig::Node::E131, remoteconfig::Output::PIXEL, bridge.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(E131MsgConst::START, Display7SegmentMessage::INFO_BRIDGE_START, CONSOLE_YELLOW);

	bridge.Start();
	llrpOnlyDevice.Start();

	display.TextStatus(E131MsgConst::STARTED, Display7SegmentMessage::INFO_BRIDGE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		bridge.Run();
		remoteConfig.Run();
		llrpOnlyDevice.Run();
		spiFlashStore.Flash();
		lb.Run();
		display.Run();
		if (__builtin_expect((pPixelTestPattern != nullptr), 0)) {
			pPixelTestPattern->Run();
		}
#if defined (ENABLE_HTTPD)
		mDns.Run();
#endif
	}
}

/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.nl
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
#include <algorithm>

#include "hardware.h"
#include "network.h"
#include "networkconst.h"
#include "storenetwork.h"
#include "ledblink.h"

#if defined (ENABLE_HTTPD)
# include "mdns.h"
# include "mdnsservices.h"
#endif

#include "displayudf.h"
#include "displayudfparams.h"
#include "displayhandler.h"
#include "artnet/displayudfhandler.h"

#include "artnet4node.h"
#include "artnetparams.h"
#include "artnetreboot.h"
#include "artnetmsgconst.h"

#include "pixeltype.h"
#include "pixeltestpattern.h"
#include "ws28xxdmxparams.h"
#include "ws28xxdmxmulti.h"
#include "ws28xxdmxstartstop.h"

#include "rdmdeviceparams.h"
#include "rdmnetdevice.h"
#include "rdmpersonality.h"
#include "rdm_e120.h"
#include "factorydefaults.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "flashrom.h"
#include "spiflashstore.h"
#include "storeartnet.h"
#include "storedisplayudf.h"
#include "storerdmdevice.h"
#include "storeremoteconfig.h"
#include "storews28xxdmx.h"

#include "firmwareversion.h"
#include "software_version.h"

using namespace artnet;

void main(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	DisplayUdf display;
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashRom spiFlashInstall;
	SpiFlashStore spiFlashStore;

	fw.Print("Ethernet Art-Net 4 Node " "\x1b[32m" "Pixel controller {4x/8x 4 Universes}" "\x1b[37m");

	hw.SetLed(hardware::LedStatus::ON);
	hw.SetRebootHandler(new ArtNetReboot);
	lb.SetLedBlinkDisplay(new DisplayHandler);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

#if defined (ENABLE_HTTPD)
	MDNS mDns;
	mDns.Start();
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_TFTP, 69);
	mDns.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=Art-Net Pixel");
	mDns.Print();
#endif

	display.TextStatus(ArtNetMsgConst::PARAMS, Display7SegmentMessage::INFO_NODE_PARMAMS, CONSOLE_YELLOW);

	PixelDmxConfiguration pixelDmxConfiguration;

	StoreWS28xxDmx storeWS28xxDmx;
	WS28xxDmxParams ws28xxparms(&storeWS28xxDmx);

	if (ws28xxparms.Load()) {
		ws28xxparms.Set(&pixelDmxConfiguration);
		ws28xxparms.Dump();
	}

	WS28xxDmxMulti pixelDmxMulti(pixelDmxConfiguration);
	pixelDmxMulti.SetPixelDmxHandler(new PixelDmxStartStop);

	const auto nActivePorts = pixelDmxMulti.GetOutputPorts();

	ArtNet4Node node(static_cast<uint8_t>(nActivePorts));

	StoreArtNet storeArtNet;
	ArtNetParams artnetParams(&storeArtNet);

	if (artnetParams.Load()) {
		artnetParams.Set(&node);
		artnetParams.Dump();
	}

	DisplayUdfHandler displayUdfHandler;
	node.SetArtNetDisplay(&displayUdfHandler);

	node.SetArtNetStore(&storeArtNet);
	node.SetOutput(&pixelDmxMulti);

	const auto nUniverses = pixelDmxMulti.GetUniverses();

	uint32_t nPortProtocolIndex = 0;

	for (uint32_t nOutportIndex = 0; nOutportIndex < nActivePorts; nOutportIndex++) {
		auto isSet = false;
		const auto nStartUniversePort = ws28xxparms.GetStartUniversePort(nOutportIndex, isSet);
		if (isSet) {
			for (uint16_t u = 0; u < nUniverses; u++) {
				node.SetUniverse(nPortProtocolIndex, lightset::PortDir::OUTPUT, static_cast<uint16_t>(nStartUniversePort + u));
				nPortProtocolIndex++;
			}
			nPortProtocolIndex = nPortProtocolIndex + static_cast<uint8_t>(ArtNet::PORTS - nUniverses);
		} else {
			nPortProtocolIndex = nPortProtocolIndex + ArtNet::PORTS;
		}
	}

	uint8_t nTestPattern;
	PixelTestPattern *pPixelTestPattern = nullptr;

	if ((nTestPattern = ws28xxparms.GetTestPattern()) != 0) {
		pPixelTestPattern = new PixelTestPattern(static_cast<pixelpatterns::Pattern>(nTestPattern), nActivePorts);
		node.SetOutput(nullptr);
	}

	char aDescription[rdm::personality::DESCRIPTION_MAX_LENGTH + 1];
	snprintf(aDescription, sizeof(aDescription) - 1, "Art-Net Pixel %d-%s:%d", nActivePorts, PixelType::GetType(WS28xxMulti::Get()->GetType()), WS28xxMulti::Get()->GetCount());

	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	const auto nLength = snprintf(aLabel, sizeof(aLabel) - 1, "Orange Pi Zero Pixel");

	RDMPersonality *pPersonalities[1] = { new RDMPersonality(aDescription, nullptr) };
	RDMNetDevice llrpOnlyDevice(pPersonalities, 1);

	llrpOnlyDevice.SetLabel(RDM_ROOT_DEVICE, aLabel, static_cast<uint8_t>(nLength));
	llrpOnlyDevice.SetProductCategory(E120_PRODUCT_CATEGORY_FIXTURE);
	llrpOnlyDevice.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	llrpOnlyDevice.SetRDMFactoryDefaults(new FactoryDefaults);

	node.SetRdmUID(llrpOnlyDevice.GetUID(), true);

	llrpOnlyDevice.Init();

	StoreRDMDevice storeRdmDevice;
	RDMDeviceParams rdmDeviceParams(&storeRdmDevice);

	if (rdmDeviceParams.Load()) {
		rdmDeviceParams.Set(&llrpOnlyDevice);
		rdmDeviceParams.Dump();
	}

	llrpOnlyDevice.SetRDMDeviceStore(&storeRdmDevice);
	llrpOnlyDevice.Print();

	node.Print();
	pixelDmxMulti.Print();

	// Display

	display.SetTitle("ArtNet Pixel 8:%dx%d", nActivePorts, WS28xxMulti::Get()->GetCount());
	display.Set(2, displayudf::Labels::IP);
	display.Set(3, displayudf::Labels::NODE_NAME);
	display.Set(4, displayudf::Labels::VERSION);
	display.Set(5, displayudf::Labels::HOSTNAME);
	display.Set(6, displayudf::Labels::UNIVERSE);
	display.Printf(7, "%s:%d G%d %s",
			PixelType::GetType(pixelDmxConfiguration.GetType()),
			pixelDmxConfiguration.GetCount(),
			pixelDmxConfiguration.GetGroupingCount(),
			PixelType::GetMap(pixelDmxConfiguration.GetMap()));

	StoreDisplayUdf storeDisplayUdf;
	DisplayUdfParams displayUdfParams(&storeDisplayUdf);

	if (displayUdfParams.Load()) {
		displayUdfParams.Set(&display);
		displayUdfParams.Dump();
	}

	display.Show(&node);

	RemoteConfig remoteConfig(remoteconfig::Node::ARTNET, remoteconfig::Output::PIXEL, node.GetActiveOutputPorts());

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	while (spiFlashStore.Flash())
		;

	display.TextStatus(ArtNetMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	node.Start();
	llrpOnlyDevice.Start();

	display.TextStatus(ArtNetMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		node.Run();
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


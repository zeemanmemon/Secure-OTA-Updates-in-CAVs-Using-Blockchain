#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include <curl/curl.h>
#include <chrono>
#include <string>

namespace veins {

/**
 * @brief
 * A tutorial demo for TraCI. When the car is stopped for longer than 10 seconds
 * it will send a message out to other cars containing the blocked road id.
 * Receiving cars will then trigger a reroute via TraCI.
 * When channel switching between SCH and CCH is enabled on the MAC, the message is
 * instead sent out on a service channel following a Service Advertisement
 * on the CCH.
 *
 * @author Christoph Sommer : initial DemoApp
 * @author David Eckhoff : rewriting, moving functionality to DemoBaseApplLayer, adding WSA
 *
 */

class VEINS_API TraCIDemo11p : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;

protected:
    simtime_t lastDroveAt;
    bool sentMessage;
    bool otaUpdateDownloaded;
    int currentSubscribedServiceId;
    std::string downloadedHash;

    double bandwidthMbps;
    double latencyMs;
    double packetLoss;
    double jitterMs;

    double simulateOTAUpdateDownload();
    double queryBlockchain(double downloadLatency, double bandwidthMbps, double latencyMs, double packetLoss, double jitterMs);
    void logDownloadResults(const std::string& nodeName, double downloadLatency, double bandwidthMbps, double latencyMs, double packetLoss, double jitterMs, double distance, double hashDuration, const std::string& downloadedHash);
    void logQueryResults(const std::string& nodeName, double downloadLatency, double queryLatency, double hashComparisonLatency, double bandwidthMbps, double latencyMs, double packetLoss, double jitterMs, double distance, const std::string& updateStatus, const std::string& downloadedHash, const std::string& blockchainHash);

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
};

} // namespace veins

#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include <curl/curl.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <random>
#include "json.hpp"
#include <openssl/sha.h>

using json = nlohmann::json;
using namespace veins;

Define_Module(veins::TraCIDemo11p);

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        otaUpdateDownloaded = false;
        currentSubscribedServiceId = -1;
        downloadedHash = "";
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);

    if (!sentMessage && !otaUpdateDownloaded) {
        sentMessage = true;
        otaUpdateDownloaded = true;

        double downloadLatency = simulateOTAUpdateDownload();
        EV << "Downloaded hash: " << downloadedHash << std::endl;
        EV << "Download latency of the OTA update package: " << downloadLatency << " seconds" << std::endl;
        EV << "Querying blockchain..." << std::endl;

        double queryLatency = queryBlockchain(downloadLatency, bandwidthMbps, latencyMs, packetLoss, jitterMs);

        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            stopService();
            delete wsm;
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());

            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            }
            else {
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
}

double TraCIDemo11p::simulateOTAUpdateDownload()
{
    double rsuX = 2000;
    double rsuY = 2000;
    double rsuZ = 3.0;

    Coord nodePosition = mobility->getPositionAt(simTime());

    double distance = sqrt(pow(nodePosition.x - rsuX, 2) + pow(nodePosition.y - rsuY, 2) + pow(nodePosition.z - rsuZ, 2));

    /* Example network scenario coverage ranges:
     - RSU Near: 300-500 meters
     - RSU Far: 500-1000 meters
     - Wi-Fi: 50 meters
     - 4G LTE: Up to 10 kilometers
     - Low-band 5G (Sub-1 GHz): Up to 10 kilometers */

    double maxBandwidthMbps = 80.0;
    double minBandwidthMbps = 15.0;
    double maxEffectiveDistance = 1000.0;
    double fileSizeMB = 40.0;

    if (distance <= maxEffectiveDistance) {
        bandwidthMbps = maxBandwidthMbps - (distance / maxEffectiveDistance) * (maxBandwidthMbps - minBandwidthMbps);
    } else {
        bandwidthMbps = minBandwidthMbps;
    }

    latencyMs = 5.0 + (distance / maxEffectiveDistance) * 35.0;
    packetLoss = 0.01 + (distance / maxEffectiveDistance) * 0.09;
    jitterMs = 1.0 + (distance / maxEffectiveDistance) * 4.0;

    double bandwidthMBps = bandwidthMbps / 8.0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> jitterDist(-jitterMs / 2.0, jitterMs / 2.0);

    double effectiveDownloadTime = (fileSizeMB / bandwidthMBps) * 1000.0;

    double totalDownloadTimeMs = effectiveDownloadTime + latencyMs;

    totalDownloadTimeMs += jitterDist(gen);

    totalDownloadTimeMs *= (1.0 + packetLoss);

    double totalDownloadTimeSec = totalDownloadTimeMs / 1000.0;

    std::uniform_int_distribution<> attackDist(0, 1); // randomly simulate MITM attack
    std::string fileName = (attackDist(gen) == 1) ? "tamperedUpdate.bin" : "OTAUpdatePackage.bin";

    if (fileName == "tamperedUpdate.bin") {
        EV << "MITM attack simulated: Downloaded file has been tampered with." << std::endl;
    }

    auto startHashTime = std::chrono::high_resolution_clock::now();

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::ifstream file(fileName, std::ifstream::binary);
    if (file.is_open()) {
        char buf[1024];
        while (file.good()) {
            file.read(buf, sizeof(buf));
            SHA256_Update(&sha256, buf, file.gcount());
        }
        file.close();
    } else {
        EV << "Failed to open " << fileName << " for hash calculation." << std::endl;
        return -1;
    }

    SHA256_Final(hash, &sha256);

    auto endHashTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> hashDuration = endHashTime - startHashTime;

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    downloadedHash = ss.str();

    logDownloadResults(getParentModule()->getFullName(), totalDownloadTimeSec, bandwidthMbps, latencyMs, packetLoss, jitterMs, distance, hashDuration.count(), downloadedHash);

    EV << getParentModule()->getFullName() << " Position: (" << nodePosition.x << ", " << nodePosition.y << ", " << nodePosition.z << ") - Distance from RSU: " << distance << " meters." << std::endl;
    EV << "Simulated OTA update download: "
       << "File Size = " << fileSizeMB << " MB, "
       << "Bandwidth = " << bandwidthMbps << " Mbps, "
       << "Latency = " << latencyMs << " ms, "
       << "Packet Loss = " << packetLoss * 100 << "%, "
       << "Jitter = " << jitterMs << " ms. "
       << "Total download time: " << totalDownloadTimeSec << " seconds." << std::endl;
    EV << "Time taken to calculate the hash: " << hashDuration.count() << " seconds." << std::endl;

    return totalDownloadTimeSec;
}


double TraCIDemo11p::queryBlockchain(double downloadLatency, double bandwidthMbps, double latencyMs, double packetLoss, double jitterMs)
{
    std::string readBuffer;
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/query?channelid=mychannel&chaincodeid=basic&function=ReadAsset&args=update1");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            EV << "Failed to query the blockchain: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    EV << "Blockchain response: " << readBuffer << std::endl;

    std::string jsonResponseStr;
    size_t jsonStart = readBuffer.find("{");
    if (jsonStart != std::string::npos) {
        jsonResponseStr = readBuffer.substr(jsonStart);
    } else {
        EV << "No valid JSON found in the response." << std::endl;
        return -1;
    }

    json jsonResponse;
    try {
        jsonResponse = json::parse(jsonResponseStr);
    } catch (json::parse_error& e) {
        EV << "Error parsing JSON response: " << e.what() << std::endl;
        return -1;
    }

    std::string blockchainHash = jsonResponse.value("hash", "");
    if (blockchainHash.empty()) {
        EV << "Hash not found in the blockchain query response." << std::endl;
        return -1;
    }

    EV << "Retrieved blockchain hash: " << blockchainHash << std::endl;

    double querySizeMB = 0.046;

    double rsuX = 2000;
    double rsuY = 2000;
    double rsuZ = 3.0;

    Coord nodePosition = mobility->getPositionAt(simTime());

    double distance = sqrt(pow(nodePosition.x - rsuX, 2) + pow(nodePosition.y - rsuY, 2) + pow(nodePosition.z - rsuZ, 2));

    double maxEffectiveDistance = 1000.0;

    if (distance <= maxEffectiveDistance) {
        bandwidthMbps = 80.0 - (distance / maxEffectiveDistance) * (80.0 - 15.0);
    } else {
        bandwidthMbps = 15.0;
    }

    latencyMs = 5.0 + (distance / maxEffectiveDistance) * 35.0;
    packetLoss = 0.01 + (distance / maxEffectiveDistance) * 0.09;
    jitterMs = 1.0 + (distance / maxEffectiveDistance) * 4.0;

    double bandwidthMBps = bandwidthMbps / 8.0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> jitterDist(-jitterMs / 2.0, jitterMs / 2.0);

    double effectiveQueryTime = (querySizeMB / bandwidthMBps) * 1000.0;

    double totalQueryTimeMs = effectiveQueryTime + latencyMs;

    totalQueryTimeMs += jitterDist(gen);

    totalQueryTimeMs *= (1.0 + packetLoss);

    double totalQueryTimeSec = totalQueryTimeMs / 1000.0;

    auto startHashComparisonTime = std::chrono::high_resolution_clock::now();

    bool isHashMatching = (downloadedHash == blockchainHash);

    auto endHashComparisonTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> hashComparisonDuration = endHashComparisonTime - startHashComparisonTime;

    std::string updateStatus = isHashMatching ? "Update not tampered, successfully installed." : "Update tampered, not installed.";

    EV << "Hash comparison result: " << (isHashMatching ? "Match" : "Mismatch") << std::endl;
    EV << updateStatus << std::endl;

    logQueryResults(getParentModule()->getFullName(), downloadLatency, totalQueryTimeSec, hashComparisonDuration.count(), bandwidthMbps, latencyMs, packetLoss, jitterMs, distance, updateStatus, downloadedHash, blockchainHash);

    return totalQueryTimeSec;
}


void TraCIDemo11p::logDownloadResults(const std::string& nodeName, double downloadLatency, double bandwidthMbps, double latencyMs, double packetLoss, double jitterMs, double distance, double hashDuration, const std::string& downloadedHash){
    std::ofstream outFile;
    outFile.open("results.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << "========================\n";
        outFile << "Download Phase:\n";
        outFile << "Node: " << nodeName << "\n";
        outFile << "Node Position: (" << mobility->getPositionAt(simTime()).x << ", " << mobility->getPositionAt(simTime()).y << ", " << mobility->getPositionAt(simTime()).z << ")\n";
        outFile << "Distance from RSU: " << distance << " meters\n";
        outFile << "Bandwidth: " << bandwidthMbps << " Mbps\n";
        outFile << "Latency: " << latencyMs << " ms\n";
        outFile << "Packet Loss: " << packetLoss * 100 << "%\n";
        outFile << "Jitter: " << jitterMs << " ms\n";
        outFile << "Download latency: " << downloadLatency << " seconds\n";
        outFile << "Hash calculation time: " << hashDuration << " seconds\n";
        outFile << "Downloaded Hash: " << downloadedHash << "\n";
        outFile << "========================\n";
        outFile.close();
    } else {
        EV << "Unable to open file for logging download results.\n";
    }
}

void TraCIDemo11p::logQueryResults(const std::string& nodeName, double downloadLatency, double queryLatency, double hashComparisonLatency, double bandwidthMbps, double latencyMs, double packetLoss, double jitterMs, double distance, const std::string& updateStatus, const std::string& downloadedHash, const std::string& blockchainHash)
{
    std::ofstream outFile;
    outFile.open("results.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << "========================\n";
        outFile << "Query Phase:\n";
        outFile << "Node: " << nodeName << "\n";
        outFile << "Node Position: (" << mobility->getPositionAt(simTime()).x << ", " << mobility->getPositionAt(simTime()).y << ", " << mobility->getPositionAt(simTime()).z << ")\n";
        outFile << "Distance from RSU: " << distance << " meters\n";
        outFile << "Bandwidth: " << bandwidthMbps << " Mbps\n";
        outFile << "Latency: " << latencyMs << " ms\n";
        outFile << "Packet Loss: " << packetLoss * 100 << "%\n";
        outFile << "Jitter: " << jitterMs << " ms\n";
        outFile << "Query latency: " << queryLatency << " seconds\n";
        outFile << "Hash comparison latency: " << hashComparisonLatency << " seconds\n";
        outFile << "Downloaded Hash: " << downloadedHash << "\n";
        outFile << "Blockchain Hash: " << blockchainHash << "\n";
        outFile << "Update Status: " << updateStatus << "\n";
        outFile << "========================\n";
        outFile.close();
    } else {
        EV << "Unable to open file for logging query results.\n";
    }
}

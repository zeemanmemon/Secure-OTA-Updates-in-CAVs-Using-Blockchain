This repository contains the implementation for my MSc research project which introduces a novel approach on securing Over-the-Air (OTA) software updates in Connected and Autonomous Vehicles (CAVs). The project integrates the VEINS simulation framework with Hyperledger Fabric to demonstrate a blockchain-backed architecture for secure OTA delivery, integrity verification, and tamper detection.

Link to dissertation (includes detailed methodology, hardware configuration, and simulation parameters): [**ProQuest Dissertation Access (University of Windsor)**](https://ledproxy2.uwindsor.ca/login?url=https://www.proquest.com/dissertations-theses/enhancing-security-over-air-updates-connected/docview/3106813877/se-2?accountid=14789)

Setup Details
-------------

**Host Environment:**

*   Oracle VM VirtualBox
    
*   Ubuntu 22.04 OS
    

Blockchain Network Setup (Hyperledger Fabric)
---------------------------------------------

1.  Overview: Visit the official getting started guide: [https://hyperledger-fabric.readthedocs.io/en/release-2.5/getting\_started.html](https://hyperledger-fabric.readthedocs.io/en/release-2.5/getting_started.html)
    
2.  Install prerequisites (Go, Docker, jq, etc.): [https://hyperledger-fabric.readthedocs.io/en/release-2.5/prereqs.html](https://hyperledger-fabric.readthedocs.io/en/release-2.5/prereqs.html)
    
3.  Install Hyperledger Fabric:

```bash
mkdir research-setup curl -sSLO https://raw.githubusercontent.com/hyperledger/fabric/main/scripts/install-fabric.sh chmod +x install-fabric.sh ./install-fabric.sh docker samples binary
```

4.  After installation, the fabric-samples directory will include ready-to-run examples.
    
5.  Use the **Asset Transfer (Basic)** sample to simulate OTA metadata creation and querying. Modify smartcontract.go and assetTransfer.go for your custom OTA logic: [https://github.com/hyperledger/fabric-samples/tree/main/asset-transfer-basic](https://github.com/hyperledger/fabric-samples/tree/main/asset-transfer-basic)
    
6.  Run the REST API server: [https://github.com/hyperledger/fabric-samples/tree/main/asset-transfer-basic/rest-api-go](https://github.com/hyperledger/fabric-samples/tree/main/asset-transfer-basic/rest-api-go)
    
7.  Use Postman to test the blockchain REST APIs:
```bash
sudo snap install postman </pre>
```

VEINS Framework Setup

1.  Follow the VEINS tutorial: [https://veins.car2x.org/tutorial/](https://veins.car2x.org/tutorial/)
    
2.  Modify the following files to match the OTA simulation logic:
    
    *   TraCIDemo11p.cc
        
    *   TraCIDemo11p.h
        
    *   OMNETConfiguration.ini
        
    *   makefrag
        
3.  Use the following commands before simulation to install dependencies and generate OTA package binaries:
```bash
cd /veins/src/veins/modules/application/traci
wget https://github.com/nlohmann/json/releases/download/v3.10.5/json.hpp
sudo apt-get update
sudo apt-get install libssl-dev

# Generate OTA update binaries
dd if=/dev/urandom of=~/veins/examples/veins/OTAUpdatePackage.bin bs=1M count=40
dd if=/dev/urandom of=~/veins/examples/veins/tamperedUpdate.bin bs=1M count=42
```

Notes
-----

*   This repo includes scripts and documentation related to my MSc thesis project.
    
*   Data was collected from simulations over WiFi, 4G, and 5G to evaluate download latency, query response times, and integrity verification using hash comparison.
    
*   For complete methodology and results, refer to the linked dissertation.

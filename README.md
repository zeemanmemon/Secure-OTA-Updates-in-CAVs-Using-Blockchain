
For my setup, I used Oracle VM VirtualBox with Ubuntu 22.04 OS.

**Blockchain Network:** 
1. Visit https://hyperledger-fabric.readthedocs.io/en/release-2.5/getting_started.html (read through this for an intro)
2. Download the prereqs to run Hyperledger Fabric from here: https://hyperledger-fabric.readthedocs.io/en/release-2.5/prereqs.html (including Go and JQ, even though they are mentioned as Optional - you will need it later).
3. Install Hyperledger Fabric: https://hyperledger-fabric.readthedocs.io/en/release-2.5/install.html. The cmds in a nutshell are:
```
mkdir research-setup
curl -sSLO https://raw.githubusercontent.com/hyperledger/fabric/main/scripts/install-fabric.sh && chmod +x install-fabric.sh
./install-fabric.sh docker samples binary
```
5. You should now have the Hyperledger Fabric latest version installed, along with the sample files.
6. For our setup, we used the basic asset transfer sample as we need to simulate creation/storage of the OTA metadata, and querying/reading of the metadata.
7. How to run the sample is mentioned here: https://github.com/hyperledger/fabric-samples/tree/main/asset-transfer-basic (to implement our functionality, we modified the smartcontract.go and assetTransfer.go).
8. How to run the REST server is mentioned here: https://github.com/hyperledger/fabric-samples/tree/main/asset-transfer-basic/rest-api-go
9. We use Postman to interact with the Blockchain REST server. Run the code below on your terminal to install Postman:
```
sudo snap install postman
```

**VEINS Framework:** 
1. Setup VEINS, and run the Veins demo scenario using the instructions here: https://veins.car2x.org/tutorial/
2. We modified the TraCIDemo11p.cc, TraCIDemo11p.h file, OMNETConfiguration.ini, and the makefrag file to fit our simulation scenario. Logic is mentioned within the files, basically, I implemented a function for each feature, i.e., downloading, querying, and logging the results obtained.
3. Make sure to run the code below in your terminal before starting the simulation:
```
cd /veins/src/veins/modules/application/traci
wget https://github.com/nlohmann/json/releases/download/v3.10.5/json.hpp
sudo apt-get update
sudo apt-get install libssl-dev
dd if=/dev/urandom of=~/veins/examples/veins/OTAUpdatePackage.bin bs=1M count=40 && dd if=/dev/urandom of=~/veins/examples/veins/tamperedUpdate.bin bs=1M count=42 (this is for creating the OTA update packages, tampered and non-tampered).



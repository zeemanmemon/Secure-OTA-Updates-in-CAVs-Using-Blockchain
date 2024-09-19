package chaincode

import (
	"encoding/json"
	"fmt"

	"github.com/hyperledger/fabric-contract-api-go/v2/contractapi"
)

// SmartContract provides functions for managing an Asset
type SmartContract struct {
	contractapi.Contract
}

// Asset describes basic details of what makes up an asset
type Asset struct {
	UpdateID    string `json:"update_id"`
	Version     string `json:"version"`
	FileName    string `json:"filename"`
	ReleaseDate string `json:"release_date"`
	Hash        string `json:"hash"`
}

// InitLedger adds a base set of assets to the ledger
func (s *SmartContract) InitLedger(ctx contractapi.TransactionContextInterface) error {
	assets := []Asset{
		{UpdateID: "update1", Version: "1.0", FileName: "update_v1.bin", ReleaseDate: "2024-07-21", Hash: "d41d8cd98f00b204e9800998ecf8427e"},
		{UpdateID: "update2", Version: "1.1", FileName: "update_v1.1.bin", ReleaseDate: "2024-08-21", Hash: "d41d8cd98f00b204e9800998ecf8427f"},
		{UpdateID: "update3", Version: "2.0", FileName: "update_v2.bin", ReleaseDate: "2024-09-21", Hash: "d41d8cd98f00b204e9800998ecf8427g"},
	}

	for _, asset := range assets {
		assetJSON, err := json.Marshal(asset)
		if err != nil {
			return err
		}

		err = ctx.GetStub().PutState(asset.UpdateID, assetJSON)
		if err != nil {
			return fmt.Errorf("failed to put to world state. %v", err)
		}
	}

	return nil
}

// CreateAsset issues a new asset to the world state with given details.
func (s *SmartContract) CreateAsset(ctx contractapi.TransactionContextInterface, updateID string, version string, filename string, releaseDate string, hash string) error {
	exists, err := s.AssetExists(ctx, updateID)
	if err != nil {
		return err
	}
	if exists {
		return fmt.Errorf("the asset %s already exists", updateID)
	}

	asset := Asset{
		UpdateID:    updateID,
		Version:     version,
		FileName:    filename,
		ReleaseDate: releaseDate,
		Hash:        hash,
	}
	assetJSON, err := json.Marshal(asset)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(updateID, assetJSON)
}

// ReadAsset returns the asset stored in the world state with given id.
func (s *SmartContract) ReadAsset(ctx contractapi.TransactionContextInterface, updateID string) (*Asset, error) {
	assetJSON, err := ctx.GetStub().GetState(updateID)
	if err != nil {
		return nil, fmt.Errorf("failed to read from world state: %v", err)
	}
	if assetJSON == nil {
		return nil, fmt.Errorf("the asset %s does not exist", updateID)
	}

	var asset Asset
	err = json.Unmarshal(assetJSON, &asset)
	if err != nil {
		return nil, err
	}

	return &asset, nil
}

// UpdateAsset updates an existing asset in the world state with provided parameters.
func (s *SmartContract) UpdateAsset(ctx contractapi.TransactionContextInterface, updateID string, version string, filename string, releaseDate string, hash string) error {
	exists, err := s.AssetExists(ctx, updateID)
	if err != nil {
		return err
	}
	if !exists {
		return fmt.Errorf("the asset %s does not exist", updateID)
	}

	// overwriting original asset with new asset
	asset := Asset{
		UpdateID:    updateID,
		Version:     version,
		FileName:    filename,
		ReleaseDate: releaseDate,
		Hash:        hash,
	}
	assetJSON, err := json.Marshal(asset)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(updateID, assetJSON)
}

// DeleteAsset deletes an given asset from the world state.
func (s *SmartContract) DeleteAsset(ctx contractapi.TransactionContextInterface, updateID string) error {
	exists, err := s.AssetExists(ctx, updateID)
	if err != nil {
		return err
	}
	if !exists {
		return fmt.Errorf("the asset %s does not exist", updateID)
	}

	return ctx.GetStub().DelState(updateID)
}

// AssetExists returns true when asset with given ID exists in world state
func (s *SmartContract) AssetExists(ctx contractapi.TransactionContextInterface, updateID string) (bool, error) {
	assetJSON, err := ctx.GetStub().GetState(updateID)
	if err != nil {
		return false, fmt.Errorf("failed to read from world state: %v", err)
	}

	return assetJSON != nil, nil
}

// GetAllAssets returns all assets found in world state
func (s *SmartContract) GetAllAssets(ctx contractapi.TransactionContextInterface) ([]*Asset, error) {
	// range query with empty string for startKey and endKey does an
	// open-ended query of all assets in the chaincode namespace.
	resultsIterator, err := ctx.GetStub().GetStateByRange("", "")
	if err != nil {
		return nil, err
	}
	defer resultsIterator.Close()

	var assets []*Asset
	for resultsIterator.HasNext() {
		queryResponse, err := resultsIterator.Next()
		if err != nil {
			return nil, err
		}

		var asset Asset
		err = json.Unmarshal(queryResponse.Value, &asset)
		if err != nil {
			return nil, err
		}
		assets = append(assets, &asset)
	}

	return assets, nil
}

#pragma once
#include<unordered_map>
#include<vector>
#include<queue>
#include"Module.h"
#include"FunctionModule.h"
#include<memory>

class ModularNetwork
{
public:
	ModularNetwork();

	template<class T>
	void addModule(std::string module_name, std::unique_ptr<T> module) {
		if (moduleMap_.find(module_name) == moduleMap_.end()) {
			if(auto* it = dynamic_cast<FunctionModuleBase*>(module.get())){
				fnModules_.push_back(std::move(module));
				moduleMap_[module_name] = fnModules_.size() - 1;
				moduleAdjOut_[fnModules_.size() - 1] = {};
				moduleAdjIn_[fnModules_.size() - 1] = {};
				std::cout << "Added Function module : " << module_name << " --(Function)--> " << fnModules_.size() - 1 << std::endl;
			}
			else if (auto* _it = dynamic_cast<StorageModuleBase*>(module.get())) {
				storageModules_.push_back(std::move(module));
				moduleMap_[module_name] = storageModules_.size() - 1;
				std::cout << "Added Storage module : " << module_name << " --(Storage)-->" << storageModules_.size() - 1 << std::endl;
			}
			else if (auto* it = dynamic_cast<ProcedureModuleBase*>(module.get())) {
				fnModules_.push_back(std::move(module));
				moduleMap_[module_name] = fnModules_.size() - 1;
				moduleAdjOut_[fnModules_.size() - 1] = {};
				moduleAdjIn_[fnModules_.size() - 1] = {};
				std::cout << "Added Function module : " << module_name << " --(Procedure)--> " << fnModules_.size() - 1 << std::endl;
			}
			else {
				std::cout << "Unknown Module type of Module : " << module_name << std::endl;
			}
		}
		else {
			std::cout << "No duplicate name allowed" << std::endl;
		}
	}

	void linkFtoFModule(std::string output_provider, std::string output_reciever, std::string param_name);
	void linkFtoPModule(std::string output_provider, std::string procedure, std::string param_name); // fn_output --(input)--> procedure
	void linkFnToStorage(const std::string& function, const std::string& storage); // fn_output --(modify)--> storage
	void linkStorageToFn(const std::string& storage, const std::string& function, const std::string& fn_param); // storage --(provide input)--> fn
	void linkProcToFn(const std::string& procedure, const std::string& function); // procedure - - - > fn (just dependency injection)
	void linkProcedureToStorage(const std::string& procedure, const std::string& storage);
	void linkPrToPr(const std::string& first, std::string& second); // dependency preservation .... 

	void executeNetwork();

	void executeNetworkLoop(bool stopCondition);

	void executeNetworkLoop(std::size_t amount);

	template<class T>
	T* storage(const std::string& storage_name) {
		const auto& it = moduleMap_.find(storage_name);
		if (it == moduleMap_.end()) {
			std::cout << "Invalid storage name" << std::endl;
			return nullptr;
		}

		auto* _storage = dynamic_cast<StorageModuleBase*>
			(storageModules_[moduleMap_[storage_name]].get());

		void* _data = _storage->dataAddress();

		if (!_data) {
			std::cout << "Corrupt/Uninitialized storage" << std::endl;
			return nullptr;
		}

		using Data = std::remove_cv_t<T>;

		if (_storage->dataType() != typeid(Data)) {
			std::cout << "Type mismatch" << std::endl;
			return nullptr;
		}

		return static_cast<Data*>(_data);
	}

private:
	std::vector<std::unique_ptr<Module>> fnModules_;
	std::vector<std::unique_ptr<Module>> storageModules_;
	std::unordered_map<std::string, std::size_t> moduleMap_;
	std::unordered_map<std::size_t, std::vector<std::size_t>> moduleAdjOut_; // out degrees
	std::unordered_map<std::size_t, std::vector<std::size_t>> moduleAdjIn_; // reverse out degrees
};


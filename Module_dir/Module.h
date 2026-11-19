#pragma once
#include<unordered_map>
#include<vector>
#include<string>
#include<any>
#include<tuple>
#include<type_traits>
#include<iostream>

class Module
{
public:
	Module() :
		outputLink_{} {
	}

	virtual ~Module() = default;

	void addOutputLink(std::string param_name, Module* module);

	const std::vector<std::tuple<std::string, Module*>>& outLinks() const {
		return outputLink_;
	}

	void addInputStorage(const std::string& input_param, Module* storage) {
		inStorageLink_.emplace_back(input_param, storage);
	}

	void addOutputStorage(Module* storage) {
		outStorageLink_.emplace_back(storage);
	}

	const std::vector<std::tuple<std::string, Module*>>& inputStorage() const {
		return inStorageLink_;
	}

	const std::vector<Module*>& outputStorage() const {
		return outStorageLink_;
	}

protected:
	std::vector<std::tuple<std::string, Module*>> outputLink_;
	std::vector<std::tuple<std::string, Module*>> inStorageLink_;
	std::vector<Module*> outStorageLink_;
};

class FunctionModuleBase : public Module {
public:
	FunctionModuleBase() = default;
	virtual ~FunctionModuleBase() = default;

	virtual void execute() = 0;
	virtual void setParameter(std::string param_name, const std::any& value) = 0;
	virtual std::any output() const = 0;

private:

};

class StorageModuleBase : public Module {
public:
	virtual void setData(const std::any& data) = 0;
	virtual std::any data() const = 0;
	virtual const std::any& const_data() const = 0;
	virtual void* dataAddress() = 0;
	virtual const std::type_info& dataType() const = 0;

private:

};

class ProcedureModuleBase : public Module { // this module will modify one or more storage elements
public:
	virtual void execute() = 0;

	virtual void setParameter(const std::string& param_name, const std::any& value) = 0;

	void addStorageLink(const std::string& storage_name, Module* module) {
		if (auto* it = dynamic_cast<StorageModuleBase*>(module)) {
			storageLinks_.emplace_back(module);
		}
	}

	const std::vector<Module*>& storageLinks() const {
		return storageLinks_;
	}

protected:
	template<class T>
	T* getStorageData(std::size_t index) {
		if (index >= storageLinks_.size()) {
			std::cout << "Invalid storage index" << std::endl;
			return nullptr;
		}

		auto* _storage = dynamic_cast<StorageModuleBase*>(storageLinks_[index]);

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
	std::vector<Module*> storageLinks_;
};

inline void Module::addOutputLink(std::string param_name, Module* module) {
	if (dynamic_cast<FunctionModuleBase*>(module) ||
		dynamic_cast<ProcedureModuleBase*>(module)) {
		outputLink_.emplace_back(
			param_name, module
		);
	}
}
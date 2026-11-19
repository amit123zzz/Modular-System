#pragma once

#include "Module.h"
#include <string>
#include<optional>
#include <iostream>

template<class Data>
class StorageModule : public StorageModuleBase {
public:
	StorageModule(const std::string& param_name) :
		variableName_{ param_name } {
	}

	void setData(const std::any& data) override {
		if (auto* it = std::any_cast<Data>(&data)) {
			data_.emplace(*it);
		}
		else {
			std::cout << "Data set error: Incompatible type" << std::endl;
		}
	}

	std::any data() const override {
		if (data_) {
			return data_.value();
		}
		else {
			std::cout << "Data is not set in the storage" << std::endl;
			return {};
		}
	}

	const std::any& const_data() const override {
		if (data_) {
			return data_.value();
		}
		else {
			std::cout << "Data is not set in the storage" << std::endl;
			return {};
		}
	}

	void* dataAddress() override {
		if (data_) {
			return static_cast<void*>(&data_.value());
		}
		return nullptr;
	}

	const std::type_info& dataType() const override {
		return typeid(Data);
	}


private:
	std::string variableName_;
	std::optional<Data> data_;
};

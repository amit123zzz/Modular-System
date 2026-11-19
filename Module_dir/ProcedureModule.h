#pragma once

#include"Module.h"
#include<functional>
#include<tuple>
#include<initializer_list>

template<class... Types>
struct StorageType {};

template<class StoragePack, class... Args>
class ProcedureModule;

template<class... StorageArgs, class... Args>
class ProcedureModule<StorageType<StorageArgs...>, Args...> 
	: public ProcedureModuleBase {
public:
	ProcedureModule(const std::initializer_list<std::string>& input_names){
		std::size_t count = sizeof...(Args);
		std::size_t inputs = input_names.size();
		if (inputs != count) {
			std::cout << "Variable name mismatch with number of variables while input" << std::endl;
		}
		std::size_t _cnt = 0;
		for (const auto& name : input_names) {
			if (_cnt >= count)break;
			inputMap_[name] = _cnt;
			_cnt++;
		}
		int _diff = static_cast<int>(_cnt) - static_cast<int>(inputs);
		if (_diff < 0) {
			std::cout << std::abs(_diff) << " amount are discarded" << std::endl;
		}

		if (_diff > 0) {
			for (std::size_t i = 0; i < _diff; ++i) {
				std::string _v = "v_" + std::to_string(i);
				inputMap_[_v] = _cnt + i;
			}
			std::cout << "Additional " << _diff << " inputs added starting from v_0 to v_" + std::to_string(_diff - 1) << std::endl;
		}
	}

	void setFunction(std::function<void(StorageArgs*..., Args...)> function) {
		func_ = std::move(function);
	}

	void execute() override {
		if (func_ && allInputPresent()) {
			auto _storageTuple = makeStorageTuple(std::index_sequence_for<StorageArgs...>{});

			auto _inputTuple = makeInputTuple();

			std::tuple _fnParams = std::tuple_cat(_storageTuple, _inputTuple);

			std::apply(
				func_, _fnParams
			);
		}
		else {
			if (!allInputPresent()) {
				std::cout << "All inputs are not present" << std::endl;
				printMissingParameters();
			}

			if (!func_) {
				std::cout << "Function is not ready" << std::endl;
			}
		}
	}

	void setParameter(const std::string& param_name, const std::any& value) override {
		setParameterHelper(param_name, value);
	}

private:
	std::unordered_map<std::string, std::size_t> inputMap_;
	std::tuple<std::optional<std::decay_t<Args>>...> inputs_;
	std::function<void(StorageArgs*..., Args...)> func_;

	template<std::size_t... I>
	auto makeStorageTuple(std::index_sequence<I...>) {
		return std::make_tuple(
			getStorageData<StorageArgs>(I)...
		);
	}

	auto makeInputTuple() {
		return std::apply(
			[](auto&... args) {
				return std::forward_as_tuple(args.value()...);
			}, inputs_
		);
	}

	bool allInputPresent() const {
		return std::apply(
			[](const auto&... args) {
				return (args.has_value() && ...);
			}, inputs_
		);
	}

	template<std::size_t I = 0, class T>
	void setParamImp(std::size_t target, T&& value) {
		if constexpr (I < sizeof...(Args)) {
			if (I == target) {
				using Value = typename std::tuple_element_t<I, decltype(inputs_)>::value_type;
				if (const auto* ptr = std::any_cast<Value>(&value)) {
					std::get<I>(inputs_).emplace(
						*ptr
					);
				}
				else {
					std::cout << "Incompatible type" << std::endl;
				}
			}
			else {
				setParamImp<I + 1>(target, std::forward<T>(value));
			}
		}
	}

	template<class T>
	void setParameterHelper(const std::string& param_name, T&& value) {
		const auto it = inputMap_.find(param_name);
		if (it != inputMap_.end()) {
			setParamImp(it->second, std::forward<T>(value));
		}
	}

	void printMissingParameters() const {
		std::apply([this](const auto&... args) {
			std::size_t index = 0;

			(([&]() {
				if (!args.has_value()) {
					for (const auto& [name, idx] : inputMap_) {
						if (idx == index) {
							std::cout << "Missing parameter: "
								<< name << '\n';
							break;
						}
					}
				}
				++index;
				}()), ...);

			}, inputs_);
	}

};

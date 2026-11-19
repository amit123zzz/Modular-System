#pragma once

#include"Module.h"
#include<functional>
#include <string_view>
#include<tuple>
#include<unordered_map>
#include<initializer_list>
#include<optional>
#include<type_traits>
#include<utility>
#include<string>
#include<iostream>

template <class Out, class... In>
class FunctionModule : public FunctionModuleBase
{
public:
	FunctionModule(std::string output_name,
		const std::initializer_list<std::string>& input_names) :
		outputName_{ output_name } {
		std::size_t count = sizeof...(In);
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

	void setFunction(std::function<Out(In...)> func) {
		function_ = std::move(func);
	}

	void setParameter(std::string param_name, const std::any& value) override {
		std::cout << "setting parameter to param " << param_name << std::endl;
		setParameterHelper(param_name, value);
	}

	void execute() override {
		executeFunction();
	}

	std::any output() const override {
		if (!output_) {
			std::cout << "Output is not ready" << std::endl;
			return {};
		}
		return output_.value();
	}

protected:
	std::function<Out(In...)> function_;
	std::string outputName_;
	std::optional<std::decay_t<Out>> output_;
	std::tuple<std::optional<std::decay_t<In>>...> inputs_;
	std::unordered_map<std::string, std::size_t> inputMap_;

private:
	bool allInputPresent() const {
		return std::apply(
			[](const auto&... args) {
				return (args.has_value() && ...);
			}, inputs_
		);
	}

	template<std::size_t I = 0, class T>
	void setParamImp(std::size_t target, T&& value) {
		if constexpr (I < sizeof...(In)) {
			if (I == target) {
				using Value = typename std::tuple_element_t<I, decltype(inputs_)>::value_type;
				if(const auto* ptr = std::any_cast<Value>(&value)) {
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

	void executeFunction() {
		if (allInputPresent() && function_) {
			output_.emplace(
				std::apply(
					[this](auto&... args) {
						return function_(args.value()...);
					}, inputs_
				)
			);
		}
		else {
			if (!allInputPresent()) {
				std::cout << "All inputs are not present" << std::endl;
				printMissingParameters();
			}

			if (!function_) {
				std::cout << "Function is not ready" << std::endl;
			}
		}
	}

	template<class T>
	void setParameterHelper(std::string param_name, T&& value) {
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

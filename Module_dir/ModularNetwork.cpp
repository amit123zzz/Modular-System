#include "ModularNetwork.h"

ModularNetwork::ModularNetwork():
	fnModules_{}, moduleMap_{}, moduleAdjOut_{}, moduleAdjIn_{}
{
}

void ModularNetwork::linkFtoFModule(std::string output_provider, std::string output_reciever, std::string param_name)
{
	// link output of a module to a input variable of a module
	const auto _op = moduleMap_.find(output_provider);
	const auto _or = moduleMap_.find(output_reciever);
	if (_op == moduleMap_.end() || _or == moduleMap_.end()) {
		std::cout << "Cannot link modules " << output_provider << " " << output_reciever << std::endl;
		return;
	}

	moduleAdjOut_[moduleMap_[output_provider]].emplace_back(moduleMap_[output_reciever]);
	moduleAdjIn_[moduleMap_[output_reciever]].emplace_back(moduleMap_[output_provider]);
	fnModules_[moduleMap_[output_provider]]->addOutputLink(param_name, fnModules_[moduleMap_[output_reciever]].get());

	std::cout << moduleMap_[output_provider] << "(F) --(" << param_name << ")--> " << moduleMap_[output_reciever] << "(F)" << std::endl;
}

void ModularNetwork::linkFtoPModule(std::string output_provider, std::string procedure, std::string param_name)
{
	const auto _op = moduleMap_.find(output_provider);
	const auto _or = moduleMap_.find(procedure);
	if (_op == moduleMap_.end() || _or == moduleMap_.end()) {
		std::cout << "Cannot link modules " << output_provider << " " << procedure << std::endl;
		return;
	}

	moduleAdjOut_[moduleMap_[output_provider]].emplace_back(moduleMap_[procedure]);
	moduleAdjIn_[moduleMap_[procedure]].emplace_back(moduleMap_[output_provider]);
	fnModules_[moduleMap_[output_provider]]->addOutputLink(param_name, fnModules_[moduleMap_[procedure]].get());

	std::cout << moduleMap_[output_provider] << "(F) --(" << param_name << ")--> " << moduleMap_[procedure] << "(P)" << std::endl;
}

void ModularNetwork::linkFnToStorage(const std::string& function, const std::string& storage)
{
	const auto _fn = moduleMap_.find(function);
	const auto _st = moduleMap_.find(storage);
	if (_fn == moduleMap_.end() || _st == moduleMap_.end()) {
		std::cout << "Cannot link modules " << function << " " << storage << std::endl;
		return;
	}

	fnModules_[moduleMap_[function]]->addOutputStorage(storageModules_[moduleMap_[storage]].get());

	std::cout << moduleMap_[function] << "(F) ----> " << moduleMap_[storage] << "(S)" << std::endl;

}

void ModularNetwork::linkStorageToFn(const std::string& storage, const std::string& function, const std::string& fn_param)
{
	const auto _fn = moduleMap_.find(function);
	const auto _st = moduleMap_.find(storage);
	if (_fn == moduleMap_.end() || _st == moduleMap_.end()) {
		std::cout << "Cannot link modules " << function << " " << storage << std::endl;
		return;
	}

	fnModules_[moduleMap_[function]]->addInputStorage(
		fn_param,
		storageModules_[moduleMap_[storage]].get()
	);

	std::cout << moduleMap_[storage] << "(S) --(" << fn_param << ")--> " << moduleMap_[function] << "(F)" << std::endl;
}

void ModularNetwork::linkProcToFn(const std::string& procedure, const std::string& function)
{
	const auto _op = moduleMap_.find(procedure);
	const auto _or = moduleMap_.find(function);
	if (_op == moduleMap_.end() || _or == moduleMap_.end()) {
		std::cout << "Cannot link modules " << procedure << " " << function << std::endl;
		return;
	}

	moduleAdjOut_[moduleMap_[procedure]].emplace_back(moduleMap_[function]);
	moduleAdjIn_[moduleMap_[function]].emplace_back(moduleMap_[procedure]);

	std::cout << moduleMap_[procedure] << "(P) - - - > " << moduleMap_[function] << "(F)" << std::endl;
}

void ModularNetwork::linkProcedureToStorage(const std::string& procedure, const std::string& storage)
{
	auto pr = moduleMap_.find(procedure);
	auto st = moduleMap_.find(storage);

	if (pr == moduleMap_.end() || st == moduleMap_.end()) {
		std::cout << "Invalid procedure or storage" << std::endl;
		return;
	}

	auto* _p_ptr = dynamic_cast<ProcedureModuleBase*>(fnModules_[moduleMap_[procedure]].get());
	auto* _s_ptr = dynamic_cast<StorageModuleBase*>(storageModules_[moduleMap_[storage]].get());

	if (!_p_ptr || !_s_ptr) {
		std::cout << "Invalid module type" << std::endl;
		return;
	}

	_p_ptr->addStorageLink(storage, storageModules_[moduleMap_[storage]].get());

	std::cout << moduleMap_[procedure] << "(P) ----> " << moduleMap_[storage] << "(S)" << std::endl;

}

void ModularNetwork::linkPrToPr(const std::string& first, std::string& second)
{
	auto _first = moduleMap_.find(first);
	auto _second = moduleMap_.find(second);

	if (_first == moduleMap_.end() || _second == moduleMap_.end()) {
		std::cout << "Invalid procedure" << std::endl;
		return;
	}

	auto* _p1_ptr = dynamic_cast<ProcedureModuleBase*>(fnModules_[moduleMap_[first]].get());
	auto* _p2_ptr = dynamic_cast<ProcedureModuleBase*>(fnModules_[moduleMap_[second]].get());

	if (!_p1_ptr || !_p2_ptr) {
		std::cout << "Invalid module type" << std::endl;
		return;
	}

	moduleAdjOut_[moduleMap_[first]].emplace_back(moduleMap_[second]);
	moduleAdjIn_[moduleMap_[second]].emplace_back(moduleMap_[first]);

	std::cout << moduleMap_[first] << "(P) - - - > " << moduleMap_[second] << "(P)" << std::endl;
}

void ModularNetwork::executeNetwork()
{
	std::vector<std::size_t> executionQueue_;

	std::queue<std::size_t> queue_;

	std::unordered_map<std::size_t, std::size_t> seq;

	std::cout << std::endl;

	std::cout << "*****(Execution Details)*****" << std::endl << std::endl;

	// first traverse the reverse out adjecency to find the freely executable modules
	for (const auto& [key, vec] : moduleAdjIn_) {
		seq[key] = vec.size();
		if (vec.empty()) {
			queue_.push(key);
		}
	}

	while (!queue_.empty()) {

		std::size_t _idx = queue_.front();
		queue_.pop();
		executionQueue_.emplace_back(_idx);

		for (auto& e : moduleAdjOut_[_idx]) {
			--seq[e];
			if (auto* _it = dynamic_cast<ProcedureModuleBase*>(fnModules_[e].get())) {
				queue_.push(e);
			}
			else {
				if (seq[e] == 0) {
					queue_.push(e);
				}
			}
		}
	}

	//if (executionQueue_.size() != fnModules_.size()) {
	//	std::cout << "Execution aborted : Circular dependency found" << std::endl;
	//	return;
	//}

	std::cout << "Execution sequence : ";

	for (const auto& s : executionQueue_) {
		std::cout << s << " ";
	}

	std::cout << std::endl;

	std::cout << "Execution queue made of size : " << executionQueue_.size() << std::endl;

	for (const auto& executive : executionQueue_) {
		if (auto* _m = dynamic_cast<FunctionModuleBase*>(fnModules_[executive].get())) {
			std::cout << std::endl;
			std::cout << "######################################################################" << std::endl;
			std::cout << executive << " Executing..." << std::endl;
			_m->execute();
			auto& _outLink = _m->outLinks();

			std::cout << "Linked Modules : ";

			for (const auto& [param, module_ptr] : _outLink) {
				for (std::size_t i{}; i < fnModules_.size(); ++i) {
					if (module_ptr == fnModules_[i].get()) {
						std::cout << i << " ";
					}
				}
			}

			std::cout << std::endl;

			std::cout << "Linked Storages : ";

			auto& _outStorage = _m->outputStorage();

			for (auto* module_ptr : _outStorage) {
				for (std::size_t i{}; i < storageModules_.size(); ++i) {
					if (module_ptr == storageModules_[i].get()) {
						std::cout << i << " ";
					}
				}
			}

			std::cout << std::endl;

			for (const auto& [param, module_ptr] : _outLink) {
				if (auto* _m_ptr = dynamic_cast<FunctionModuleBase*>(module_ptr)) {
					_m_ptr->setParameter(param, _m->output());
				}
				else if (auto* _p_ptr = dynamic_cast<ProcedureModuleBase*>(module_ptr)) {
					_p_ptr->setParameter(param, _m->output());
				}
			}

			for (auto* module_ptr : _outStorage) {
				if (auto* _m_ptr = dynamic_cast<StorageModuleBase*>(module_ptr)) {
					_m_ptr->setData(_m->output());
				}
			}

			std::cout << executive << " module execution complete." << std::endl;
		}
		else if (auto* ptr = dynamic_cast<ProcedureModuleBase*>(fnModules_[executive].get())) {
			std::cout << std::endl;
			std::cout << "######################################################################" << std::endl;
			std::cout << executive << " Executing..." << std::endl;

			ptr->execute();

			std::cout << executive << " module execution complete." << std::endl;

		}
		else {
			std::cout << executive << " module execution skipped." << std::endl;
		}

		std::cout << "######################################################################" << std::endl;
	}

	std::cout << std::endl;

	std::cout << "*****(Execution Ended)*****" << std::endl << std::endl;
}

void ModularNetwork::executeNetworkLoop(bool stopCondition)
{
	std::vector<std::size_t> executionQueue_;

	std::queue<std::size_t> queue_;

	std::unordered_map<std::size_t, std::size_t> seq;

	std::cout << std::endl;

	std::cout << "*****(Execution Details)*****" << std::endl << std::endl;

	// first traverse the reverse out adjecency to find the freely executable modules
	for (const auto& [key, vec] : moduleAdjIn_) {
		seq[key] = vec.size();
		if (vec.empty()) {
			queue_.push(key);
		}
	}

	while (!queue_.empty()) {

		std::size_t _idx = queue_.front();
		queue_.pop();
		executionQueue_.emplace_back(_idx);

		for (auto& e : moduleAdjOut_[_idx]) {
			--seq[e];
			if (auto* _it = dynamic_cast<ProcedureModuleBase*>(fnModules_[e].get())) {
				queue_.push(e);
			}
			else {
				if (seq[e] == 0) {
					queue_.push(e);
				}
			}
		}
	}

	//if (executionQueue_.size() != fnModules_.size()) {
	//	std::cout << "Execution aborted : Circular dependency found" << std::endl;
	//	return;
	//}

	std::cout << "Execution sequence : ";

	for (const auto& s : executionQueue_) {
		std::cout << s << " ";
	}

	std::cout << std::endl;

	std::cout << "Execution queue made of size : " << executionQueue_.size() << std::endl;

	while (!stopCondition) {
		for (const auto& executive : executionQueue_) {
			if (auto* _m = dynamic_cast<FunctionModuleBase*>(fnModules_[executive].get())) {
				std::cout << std::endl;
				std::cout << "######################################################################" << std::endl;
				std::cout << executive << " Executing..." << std::endl;

				auto& _inStorage = _m->inputStorage();

				for (const auto& [param_name, module_ptr] : _inStorage) {
					if (auto* _m_ptr = dynamic_cast<StorageModuleBase*>(module_ptr)) {
						_m->setParameter(param_name, _m_ptr->const_data());
					}
				}

				_m->execute();
				auto& _outLink = _m->outLinks();

				std::cout << "Linked Modules : ";

				for (const auto& [param, module_ptr] : _outLink) {
					for (std::size_t i{}; i < fnModules_.size(); ++i) {
						if (module_ptr == fnModules_[i].get()) {
							std::cout << i << " ";
						}
					}
				}

				std::cout << std::endl;

				std::cout << "Linked Storages : ";

				auto& _outStorage = _m->outputStorage();

				for (auto* module_ptr : _outStorage) {
					for (std::size_t i{}; i < storageModules_.size(); ++i) {
						if (module_ptr == storageModules_[i].get()) {
							std::cout << i << " ";
						}
					}
				}

				std::cout << std::endl;

				for (const auto& [param, module_ptr] : _outLink) {
					if (auto* _m_ptr = dynamic_cast<FunctionModuleBase*>(module_ptr)) {
						_m_ptr->setParameter(param, _m->output());
					}
					else if (auto* _p_ptr = dynamic_cast<ProcedureModuleBase*>(module_ptr)) {
						_p_ptr->setParameter(param, _m->output());
					}
				}

				for (auto* module_ptr : _outStorage) {
					if (auto* _m_ptr = dynamic_cast<StorageModuleBase*>(module_ptr)) {
						_m_ptr->setData(_m->output());
					}
				}

				std::cout << executive << " module execution complete." << std::endl;
			}
			else if (auto* ptr = dynamic_cast<ProcedureModuleBase*>(fnModules_[executive].get())) {
				std::cout << std::endl;
				std::cout << "######################################################################" << std::endl;
				std::cout << executive << " Executing..." << std::endl;

				ptr->execute();


				std::cout << executive << " module execution complete." << std::endl;

			}
			else {
				std::cout << executive << " module execution skipped." << std::endl;
			}
			std::cout << "######################################################################" << std::endl;
		}
	}

	std::cout << std::endl;


	std::cout << "*****(Execution Ended)*****" << std::endl << std::endl;

}

void ModularNetwork::executeNetworkLoop(std::size_t amount)
{
	std::vector<std::size_t> executionQueue_;

	std::queue<std::size_t> queue_;

	std::unordered_map<std::size_t, std::size_t> seq;

	std::cout << std::endl;

	std::cout << "*****(Execution Details)*****" << std::endl << std::endl;

	// first traverse the reverse out adjecency to find the freely executable modules
	for (const auto& [key, vec] : moduleAdjIn_) {
		seq[key] = vec.size();
		if (vec.empty()) {
			queue_.push(key);
		}
	}

	while (!queue_.empty()) {

		std::size_t _idx = queue_.front();
		queue_.pop();
		executionQueue_.emplace_back(_idx);

		for (auto& e : moduleAdjOut_[_idx]) {
			--seq[e];
			if (auto* _it = dynamic_cast<ProcedureModuleBase*>(fnModules_[e].get())) {
				queue_.push(e);
			}
			else {
				if (seq[e] == 0) {
					queue_.push(e);
				}
			}
		}
	}

	//if (executionQueue_.size() != fnModules_.size()) {
	//	std::cout << "Execution aborted : Circular dependency found" << std::endl;
	//	return;
	//}

	std::cout << "Execution sequence : ";

	for (const auto& s : executionQueue_) {
		std::cout << s << " ";
	}

	std::cout << std::endl;

	std::cout << "Execution queue made of size : " << executionQueue_.size() << std::endl;

	for(std::size_t i = 0; i < amount; ++i) {
		for (const auto& executive : executionQueue_) {
			if (auto* _m = dynamic_cast<FunctionModuleBase*>(fnModules_[executive].get())) {
				std::cout << std::endl;
				std::cout << "######################################################################" << std::endl;
				std::cout << executive << " Executing..." << std::endl;
				_m->execute();
				auto& _outLink = _m->outLinks();

				std::cout << "Linked Modules : ";

				for (const auto& [param, module_ptr] : _outLink) {
					for (std::size_t i{}; i < fnModules_.size(); ++i) {
						if (module_ptr == fnModules_[i].get()) {
							std::cout << i << " ";
						}
					}
				}

				std::cout << std::endl;

				std::cout << "Linked Storages : ";

				auto& _outStorage = _m->outputStorage();

				for (auto* module_ptr : _outStorage) {
					for (std::size_t i{}; i < storageModules_.size(); ++i) {
						if (module_ptr == storageModules_[i].get()) {
							std::cout << i << " ";
						}
					}
				}

				std::cout << std::endl;

				for (const auto& [param, module_ptr] : _outLink) {
					if (auto* _m_ptr = dynamic_cast<FunctionModuleBase*>(module_ptr)) {
						_m_ptr->setParameter(param, _m->output());
					}
					else if (auto* _p_ptr = dynamic_cast<ProcedureModuleBase*>(module_ptr)) {
						_p_ptr->setParameter(param, _m->output());
					}
				}

				for (auto* module_ptr : _outStorage) {
					if (auto* _m_ptr = dynamic_cast<StorageModuleBase*>(module_ptr)) {
						_m_ptr->setData(_m->output());
					}
				}

				std::cout << executive << " module execution complete." << std::endl;
			}
			else if (auto* ptr = dynamic_cast<ProcedureModuleBase*>(fnModules_[executive].get())) {
				std::cout << std::endl;
				std::cout << "######################################################################" << std::endl;
				std::cout << executive << " Executing..." << std::endl;

				ptr->execute();

				std::cout << executive << " module execution complete." << std::endl;

			}
			else {
				std::cout << executive << " module execution skipped." << std::endl;
			}
			std::cout << "######################################################################" << std::endl;
		}

	}

	std::cout << std::endl;

	std::cout << "*****(Execution Ended)*****" << std::endl << std::endl;
}

#include"Module_dir/ModularNetwork.h"
#include "Module_dir/VariableStorageModule.h"
#include "Module_dir/ProcedureModule.h"

int main() {

	auto addition = std::make_unique<FunctionModule<int, int, int>>(
		std::string("out"), std::initializer_list<std::string>{ "a", "b" }
	);
	addition->setFunction(
		[](int a, int b) {
			return a + b;
		}
	);
	addition->setParameter("a", 100);
	addition->setParameter("b", 200);

	auto addition1 = std::make_unique<FunctionModule<int, int, int>>(*addition);
	addition1->setParameter("a", 10);
	addition1->setParameter("b", 22);

	auto difference = std::make_unique<FunctionModule<int, int, int>>(
		std::string("out"), std::initializer_list<std::string>{"a", "b"}
	);
	difference->setFunction(
		[](int a, int b) {
			return a - b;
		}
	);

	auto multiply = std::make_unique<FunctionModule<int, int, int>>(
		std::string("out"), std::initializer_list<std::string>{"a", "b"}
	);
	multiply->setFunction(
		[](int a, int b) {
			return a * b;
		}
	);
	multiply->setParameter("a", 2);

	auto divide = std::make_unique<FunctionModule<int, int, int>>(
		std::string("out"), std::initializer_list<std::string>{"a", "b"}
	);
	divide->setFunction(
		[](int a, int b) {
			return a / b;
		}
	);

	auto printNumber = std::make_unique<FunctionModule<int, int>>(          
		std::string("null"), std::initializer_list<std::string>{"input"}
	);
	printNumber->setFunction(
		[](int num) {
			std::cout << num << std::endl;
			return 0;
		}
	);

	auto numStorage = std::make_unique<StorageModule<int>>(std::string("num"));
	numStorage->setData(0);

	auto additionProcedure = std::make_unique<ProcedureModule<StorageType<int>, int>>(
		std::initializer_list<std::string>{ "amount" }
	);
	additionProcedure->setFunction(
		[](int* num, int a) {
			if(num) *num += a;
		}
	);

	ModularNetwork _network;
	_network.addModule("addition1", std::move(addition));
	_network.addModule("addition2", std::move(addition1));
	_network.addModule("subtract", std::move(difference));
	_network.addModule("multiply", std::move(multiply));
	_network.addModule("divide", std::move(divide));
	_network.addModule("print", std::move(printNumber));
	_network.addModule("num", std::move(numStorage));
	_network.addModule("Increase", std::move(additionProcedure));

	_network.linkFtoFModule("addition1", "subtract", "a");
	_network.linkFtoFModule("addition2", "multiply", "b");
	_network.linkFtoFModule("subtract", "divide", "a");
	_network.linkFtoFModule("addition2", "divide", "b");
	_network.linkFtoFModule("multiply", "subtract", "b");
	_network.linkFtoFModule("divide", "print", "input");
	_network.linkFtoPModule("divide", "Increase", "amount");
	_network.linkFtoPModule("addition1", "Increase", "amount");
	_network.linkProcedureToStorage("Increase", "num");

	_network.executeNetworkLoop(static_cast<std::size_t>(5));

	int _num = *_network.storage<int>("num");
	std::cout << _num << std::endl;

	return 0;
}

/*
	
	1. ModuleManager will be named as ModulerNetwork
	2. ModulerNetwork will have own storage collection that can be modified by it self
	3. There will be no Module manager, there will be only ModuleExecutor
	4. ModuleExecutor will execute ModulerNetwork
	5. ModuleExecutor can execute Module sequentially or can execute them like graph higherarchy
	6. There will be a conditional loop module and a counting loop module that will run a network multiple times
	7. Maybe conditional branch.......

*/
#include <pybind11/embed.h>
#include <iostream>
#include <unordered_map>


// compile with 
// g++ pythonOne.cpp  -fPIC -I/home/gabriel/.local/lib/python3.10/site-packages/pybind11/include -I/usr/include/python3.10 -lpython3.10 -std=c++20

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{};  // Start the Python interpreter

    try {
        // Import your Python script
        py::module example = py::module::import("moddd");

        // Call the Python function
        py::dict result = example.attr("return_dictionary")();

        // Convert the Python dictionary to a C++ unordered_map
        std::unordered_map<std::string, py::object> cppDict;
        for (auto item : result) {
            cppDict[item.first.cast<std::string>()] = py::cast<py::object>(item.second);
        }

        // Process the C++ unordered_map
        for (const auto& entry : cppDict) {
            std::cout << "Key: " << entry.first << ", Value: ";

            // Handle different value types as needed
            if (py::isinstance<py::int_>(entry.second)) {
                std::cout << entry.second.cast<int>();
            } else if (py::isinstance<py::str>(entry.second)) {
                std::cout << entry.second.cast<std::string>();
            } else {
                std::cout << "Unsupported type";
            }

            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

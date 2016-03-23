#pragma once

// Includes
#include <memory>
#include <iostream>
#include <exception>

#include <unordered_map>
#include <string>
#include <vector>

#include <chrono>
#include <atomic>
#include <functional>

#include <sstream>
#include <thread>
#include <random>

#include <stdint.h>
#include <signal.h>
#include <unistd.h>

#include <dirent.h>
#include <sstream>

#include <fstream>
#include <cstring>

// 3rd party libraries
#include "../../Libraries/EasyLogging/easylogging++.h"
#include "../../Libraries/Opencv3.1/include/opencv2/opencv.hpp"

#define PRINT_VAR(x) #x << '=' << x

namespace util
{
	// Correlary to make_shared
	template<typename T, typename ...Args>
	std::unique_ptr<T> make_unique( Args&& ...args )
	{
		return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
	}

}


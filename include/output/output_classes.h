#pragma once

#include <cstring>
#include <iostream>
#include <memory>

#include "../definitions.h"

#include "../argparser.h"
#include "barcode.h"
#include "base.h"
#include "betti.h"
#include "trivial.h"

#ifdef WITH_HDF5
#include "barcode_hdf5.h"
#endif

/* issues with compiler version and std::make_unique were discovered
 * see: https://github.com/luetge/flagser/pull/20
 */
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

// aparently clang defines also gnuc as macros, need to check first for clang
#if !defined(__clang__) && (defined(__GNUC__) || defined(__GNUG__)) && GCC_VERSION < 40900
template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

bool has_zero_filtration_and_no_explicit_output(const named_arguments_t& named_arguments) {
	return strlen(get_argument_or_default(named_arguments, "out-format", "")) == 0 &&
	       std::string(get_argument_or_default(named_arguments, "filtration", "zero")) == "zero";
}

template <typename Complex> std::unique_ptr<output_t<Complex>> get_output(const named_arguments_t& named_arguments) {
    // using std namespace because of std::make_unique workaround
    // This restrict the scope to inside the function
    using namespace std;

	std::string output_name = "barcode";
	auto it = named_arguments.find("out-format");
	if (it != named_arguments.end()) { output_name = it->second; }

	if (output_name == "betti" || has_zero_filtration_and_no_explicit_output(named_arguments))
		return make_unique<betti_output_t<Complex>>(named_arguments);
	if (output_name == "barcode") return make_unique<barcode_output_t<Complex>>(named_arguments);

	if (output_name == "none") return make_unique<trivial_output_t<Complex>>(named_arguments);

#ifdef WITH_HDF5
	if (output_name == "barcode:hdf5") return make_unique<barcode_hdf5_output_t<Complex>>(named_arguments);
#endif

	std::cerr << "The output format \"" << output_name << "\" could not be found." << std::endl;
	exit(1);
}

std::vector<std::string> available_output_formats = {"barcode", "betti"
#ifdef WITH_HDF5
                                                     ,
                                                     "barcode:hdf5"
#endif
};

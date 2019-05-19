#pragma once

#include "shared_EXPORTS.h"
#include <cstddef>

namespace async {

using handle_t = void *;

handle_t SHARED_EXPORT connect(std::size_t bulk);
void SHARED_EXPORT receive(handle_t handle, const char *data, std::size_t size);
void SHARED_EXPORT disconnect(handle_t handle);

}

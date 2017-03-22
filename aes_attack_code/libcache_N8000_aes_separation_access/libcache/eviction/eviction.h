/* See LICENSE file for license and copyright information */

#ifndef LIBCACHE_EVICTION_H
#define LIBCACHE_EVICTION_H

#include "../libcache.h"

bool libcache_eviction_init(libcache_session_t* session, libcache_session_args_t* args);
bool libcache_eviction_terminate(libcache_session_t* session);
void libcache_eviction_evict(libcache_session_t* session, void* address);

void libcache_eviction_prime(libcache_session_t* session, size_t set_index);
uint64_t libcache_eviction_probe(libcache_session_t* session, size_t set_index);

size_t libcache_eviction_get_set_index(libcache_session_t* session, void* address);
size_t libcache_eviction_get_number_of_sets(libcache_session_t* session);

#endif // libcache_EVICTION_H

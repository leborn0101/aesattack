/* See LICENSE file for license and copyright information */

#ifndef LIBCACHE_H
#define LIBCACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * libcache session
 */
typedef struct libcache_session_args_s {
  size_t bind_to_cpu; /**< CPU id to bind dedicated thread timer */
  bool performance_register_div64; /**< Enable 64 divisor (ARM only) */
} libcache_session_args_t;

typedef struct libcache_session_s libcache_session_t;

/**
 * Initializes the libcache session
 *
 * @param[out] session The initialized session
 * @param[in] args Additional arguments for the initialization
 *
 * @return true Initialization was successful
 * @return false Initialization failed
 */
bool libcache_init(libcache_session_t** session, libcache_session_args_t* args);

/**
 * Terminates the libcache session
 *
 * @param[in] session The initialized session
 *
 * @return true Termination was successful
 * @return false Termination failed
 */
bool libcache_terminate(libcache_session_t* session);

/**
 * Eviction the set correlation with address
 *
 * @param[in] session The used session
 * @param[in] address The eviction address
 */
void libcache_flush(libcache_session_t* session, void* address);

uint64_t libcache_get_basetime(libcache_session_t* session);

/**
 * Return libcache_flush time
 *
 * @param[in] session The used session
 *
 * @param[in] address The flush address
 *
 * @return time Cost to flush address  	
 */
uint64_t libcache_flush_time(libcache_session_t* session, void* address);

/**
 * Get current time measurement
 *
 * @param[in] session The used session
 *
 * @return Current time measurements
 */
uint64_t libcache_get_timing(libcache_session_t* session);

/**
 * Get current time
 *
 * @param[in] session The used session
 *
 * @return Current time
 */
uint64_t libcache_get_timing_start(libcache_session_t* session);

/**
 * Get current time
 *
 * @param[in] session The used session
 *
 * @return Current time
 */
uint64_t libcache_get_timing_end(libcache_session_t* session);

/**
 * Accesses the given data pointer
 *
 * @param[in] address Address to access
 */
void libcache_access_memory(void *address);

/**
 * Memory barrier
 */
void libcache_memory_barrier();

/**
 * Primes a given cache set.
 *
 * @param[in] session The used session
 * @param[in] set_index The set index
 */
void libcache_prime(libcache_session_t* session, size_t set_index);

/**
 * Probes a given cache set.
 *
 * @param[in] session The used session
 * @param[in] set_index The set index
 *
 * @return Timing measurement
 */
uint64_t libcache_probe(libcache_session_t* session, size_t set_index);

/**
 * Returns the set index of a given address
 *
 * @param[in] session The used session
 * @param[in] address The target address
 *
 * @return The set index
 */
size_t libcache_get_set_index(libcache_session_t* session, void* address);

/**
 * Evict a given address
 * 
 * @param[in] session The used session
 * @param[in] address The target address 
 */
void libcache_evict(libcache_session_t* session, void* address);

/**
 * Returns the time of evict a giving address
 *
 * @param[in] session The used session
 *
 * @param[in] address The target address
 *
 * @return The cost time
 */
uint64_t libcache_evict_time(libcache_session_t* session, void* address);

/**
 * Returns The time of reload a given address
 *
 * @param[in] session The used session
 *
 * @param[in] The target adress
 *
 * @return The time
 */
uint64_t libcache_reload_address(libcache_session_t* session, void* address);

/**
 * Returns The time of reload given address then eviction
 * 
 * @param[in] session The used session
 *
 * @param[in] The target address
 *
 * @return The time
 */
uint64_t libcache_reload_address_and_evict(libcache_session_t* session, void* address);

/**
 * Returns The time of reload given address then eviction
 * 
 * @param[in] session The used session
 *
 * @param[in] The reload address
 * @param[in] The evict address
 *
 * @return The time
 */
uint64_t libcache_reload_address_and_evict_other_address(libcache_session_t* session, void* address1, void* address2);
/**
 * Returns the number of sets
 *
 * @param[in] session The used session
 *
 * @return The number of sets
 */
size_t libcache_get_number_of_sets(libcache_session_t* session);

/**
 * Returns the physical address of an virtual address.
 *
 * @param[in] session The used session
 * @param[in] virtual_address The virtual address
 *
 * @return The physical address
 */
uintptr_t libcache_get_physical_address(libcache_session_t* session, uintptr_t virtual_address);

/**
 * Returns the raw pagemap entry of an virtual address.
 *
 * @param[in] session The used session
 * @param[in] virtual_address The virtual address
 *
 * @return The raw pagemap entry
 */
uint64_t libcache_get_pagemap_entry(libcache_session_t* session, uint64_t virtual_address);

/**
 * Binds the process to a cpu
 *
 * @param[in] cpu The cpu id
 *
 * @return true Binding to the cpu was successful
 * @return false Binding to the cpu failed
 */
bool libcache_bind_to_cpu(size_t cpu);

#ifdef __cplusplus
}
#endif

#endif /* LIBCACHE_H */

#ifndef HYPERION_APP_REGISTRY_H_
#define HYPERION_APP_REGISTRY_H_


#include "hyperion_app_test.h"
#include "hyperion_app_slave.h"

typedef union {
	hyperion_app_test_context_t test;
	hyperion_app_slave_context_t slave;
} hyperion_app_contexts_t;

extern hyperion_app_contexts_t hyperion_app_contexts;

#endif /* HYPERION_APP_REGISTRY_H_ */

#ifndef CENX4_APP_REGISTRY_H_
#define CENX4_APP_REGISTRY_H_

#include "cenx4_app_setup.h"
#include "cenx4_app_ableton.h"
#include "cenx4_app_test.h"
#include "cenx4_app_cfg.h"
#include "cenx4_app_log.h"
#include "cenx4_app_traktor.h"

typedef union {
    cenx4_app_setup_context_t setup;
    cenx4_app_ableton_context_t ableton;
    cenx4_app_test_context_t test;
    cenx4_app_log_viewer_context_t log_viewer;
    cenx4_app_traktor_context_t traktor;
} cenx4_app_contexts_t;

extern cenx4_app_contexts_t cenx4_app_contexts;


#endif /* CENX4_APP_REGISTRY_H_ */

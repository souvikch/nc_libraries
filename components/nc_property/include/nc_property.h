#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "nc_common.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { NC_PROPERTY_BOOL, NC_PROPERTY_I32, NC_PROPERTY_U32, NC_PROPERTY_F32, NC_PROPERTY_STRING } nc_property_type_t;
typedef union { bool boolean; int32_t i32; uint32_t u32; float f32; const char *string; } nc_property_value_t;
typedef struct { const char *name; nc_property_type_t type; bool readable; bool writable; const char *unit; } nc_property_descriptor_t;
typedef struct { nc_property_descriptor_t descriptor; nc_property_value_t value; bool value_valid; } nc_property_t;
nc_err_t nc_property_validate(const nc_property_t *property);
#ifdef __cplusplus
}
#endif

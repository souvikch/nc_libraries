#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { NC_OK = 0, NC_ERR_INVALID_ARG, NC_ERR_NOT_INITIALIZED, NC_ERR_NOT_FOUND, NC_ERR_NOT_SUPPORTED, NC_ERR_IO, NC_ERR_NO_MEMORY, NC_ERR_BUSY, NC_ERR_INTERNAL } nc_err_t;
const char *nc_err_to_name(nc_err_t error);
#ifdef __cplusplus
}
#endif

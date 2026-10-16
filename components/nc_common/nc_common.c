#include "nc_common.h"
const char *nc_err_to_name(nc_err_t error)
{
    switch (error) {
    case NC_OK: return "NC_OK"; case NC_ERR_INVALID_ARG: return "NC_ERR_INVALID_ARG";
    case NC_ERR_NOT_INITIALIZED: return "NC_ERR_NOT_INITIALIZED"; case NC_ERR_NOT_FOUND: return "NC_ERR_NOT_FOUND";
    case NC_ERR_NOT_SUPPORTED: return "NC_ERR_NOT_SUPPORTED"; case NC_ERR_IO: return "NC_ERR_IO";
    case NC_ERR_NO_MEMORY: return "NC_ERR_NO_MEMORY"; case NC_ERR_BUSY: return "NC_ERR_BUSY";
    case NC_ERR_INTERNAL: return "NC_ERR_INTERNAL"; default: return "NC_ERR_UNKNOWN";
    }
}

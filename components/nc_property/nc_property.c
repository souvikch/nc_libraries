#include "nc_property.h"
nc_err_t nc_property_validate(const nc_property_t *property)
{
    if (property == 0 || property->descriptor.name == 0 || property->descriptor.name[0] == '\0') return NC_ERR_INVALID_ARG;
    if (property->descriptor.type < NC_PROPERTY_BOOL || property->descriptor.type > NC_PROPERTY_STRING) return NC_ERR_INVALID_ARG;
    return NC_OK;
}

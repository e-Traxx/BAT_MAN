#ifndef ROBIN_TYPES_H
#define ROBIN_TYPES_H

// Error codes
typedef enum {
    PARSER_OK = 0,
    PARSER_ERROR_PEC_MISMATCH,
    PARSER_ERROR_NULL_POINTER,
    PARSER_ERROR_BOUNDS_EXCEEDED
} parser_error_t;

#endif // ROBIN_TYPES_H 
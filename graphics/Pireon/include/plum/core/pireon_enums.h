#ifndef PIREON_TYPES_H
#define PIREON_TYPES_H

typedef enum {
    PR_FORMAT_RGBA8,
    PR_FORMAT_DEPTH32
} pr_format;

typedef enum {
    PR_QUEUE_GRAPHICS,
    PR_QUEUE_COMPUTE
} pr_queue_type;


#endif 
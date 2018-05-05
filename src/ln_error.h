#ifndef _LN_ERROR_H_
#define _LN_ERROR_H_

typedef enum ln_error_level ln_error_level;
enum ln_error_level {
     LN_ERROR,
     LN_ERROR_SYS,
     LN_WARNING,
     LN_WARNING_SYS,
     LN_INFO
};

typedef struct ln_error ln_error;
struct ln_error {
     char            *err_str;
     ln_error_level   level;
};

#ifdef __cplusplus
extern "C" {
#endif

ln_error *ln_error_create(ln_error_level level, const char *fmt, ...);
void ln_error_free(ln_error *error);
void ln_error_handle(ln_error **error);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_ERROR_H_ */

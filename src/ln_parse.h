#ifndef _LN_PARSE_H_
#define _LN_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

ln_list *ln_parse_ops(const char * const json_str,
		      const ln_list *registered_ops, ln_error **error);
#ifdef __cplusplus
}
#endif

#endif	/* _LN_PARSE_H_ */

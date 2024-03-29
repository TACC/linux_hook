#ifndef __HOOK
#define __HOOK

enum ERROR_CODE_REGISTER_HOOK{REGISTER_SUCCESS, REGISTER_NOT_FOUND_LIBC, REGISTER_DLOPEN_FAILED, \
	REGISTER_MODULE_NAME_TOO_LONG, REGISTER_FUNC_NAME_TOO_LONG, REGISTER_TOO_MANY_HOOKS };

/*
 * register_a_hook - Add one target function into the list of the functions to intercept.
 *   @module_name: The name of shared library. Both short name ("ld") and full name ("ld-2.17.so") are accepted. 
 *   @func_Name:   The function name. 
 *   @new_func_addr: The address of our new implementation. 
 *   @ptr_org_func: *ptr_org_func will hold the address of orginal function implemented in lib module_name. 
 * Returns:
 *   0: success; otherwise fail. 
 */
int register_a_hook(const char *module_name, const char *func_Name, const void *new_func_addr, const long int *ptr_org_func);

/*
 * install_hook - Install hooks by setting up trampolines for all functions registered.
 * Returns:
 *   The number of hooks actually installed. 
 */
int install_hook(void);

/*
 * uninstall_hook - Uninstall hooks by cleaning up trampolines.
 * Returns:
 *   void
 */
void uninstall_hook(void);

#endif


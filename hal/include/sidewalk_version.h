#include <stdint.h>
#include <stddef.h>

extern const char *const sidewalk_version_common_commit;
extern const size_t sidewalk_version_component_count;
extern const char *const build_time_stamp;

extern const char *const sidewalk_version_component_name[];
extern const char *const sidewalk_version_component[];

#define NAME_COLUMN_WIDTH "20"

#define PRINT_SIDEWALK_VERSION(print_function)						      \
	print_function("----------------------------------------------------------------\n"); \
	for (int i = 0; i < sidewalk_version_component_count; i++)			      \
	{ print_function("%-"NAME_COLUMN_WIDTH "s %s\n", sidewalk_version_component_name[i],  \
			 sidewalk_version_component[i]); }				      \
	print_function("----------------------------------------------------------------\n"); \
	print_function("sidewalk_fork_point = %s \n", sidewalk_version_common_commit);	      \
	print_function("build time          = %s\n", build_time_stamp);			      \
	print_function("----------------------------------------------------------------\n\n")

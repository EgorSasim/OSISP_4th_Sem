#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#define MAX_PATH 4096
#define MSG_SIZE 4096
#define BUFFER_SIZE 1024*1024*16

static char *g_pModuleName;
static uid_t g_uid;
static gid_t *g_gids;
static int g_gidCount = 0;

struct fileinfo_s
{
	char name[MAX_PATH];
	char *basename;
	size_t size;
} typedef fileinfo_t;

void sys_error( const char *text, ... )
{
	static char tmp[MSG_SIZE];

	va_list argptr;
	va_start(argptr, text);
	vsnprintf(tmp, sizeof tmp, text, argptr );
	va_end(argptr);

	fprintf(stderr, "%s: ", g_pModuleName);
	perror(tmp);
}

void sort_by_name( fileinfo_t **files, int file_count )
{
	fileinfo_t *tmp;

	for( int i = 0; i < file_count; i++ )
	{
		for( int j = i+1; j < file_count; j++ )
		{
			if( strcmp(files[i]->basename, files[j]->basename ) > 0 )
			{
				tmp = files[i];
				files[i] = files[j];
				files[j] = tmp;
			}
		}
	}
}

void sort_by_size( fileinfo_t **files, int file_count )
{
	fileinfo_t *tmp;

	for( int i = 0; i < file_count; i++ )
	{
		for( int j = i+1; j < file_count; j++ )
		{
			if( files[i]->size > files[j]->size )
			{
				tmp = files[i];
				files[i] = files[j];
				files[j] = tmp;
			}
		}
	}
}


fileinfo_t **recursive_find( fileinfo_t **files, int *file_count, char *cur_path )
{
	char path[MAX_PATH];
	static const int FILE_BASE_COUNT = 1024;
	static int k = 1;

	if( files == NULL )
	{
		files = (fileinfo_t**)malloc(sizeof(fileinfo_t*)*FILE_BASE_COUNT);
		k = 1;
	}

	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(cur_path)))
	{
		sys_error("opendir failed for %s", cur_path);
		return files;
	}

	for(;;)
	{
		errno = 0;
		entry = readdir(dir);
		if( entry == NULL )
		{
			if( errno == -1 ) sys_error("readdir failed for %s", cur_path);
			break;
		}

		if (entry->d_type == DT_DIR)
		{
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			snprintf(path, sizeof(path), "%s/%s", cur_path, entry->d_name);
			files = recursive_find( files, file_count, path );
		}
		else if(entry->d_type == DT_REG)
		{
			snprintf(path, MAX_PATH, "%s/%s", cur_path, entry->d_name);

			struct stat st;
			if( stat(path, &st) == -1 )
			{
				sys_error("stat failed for file %s", path);
				continue;
			}

			bool bCanRead = (st.st_mode & S_IROTH) ||
					(st.st_uid == g_uid && (st.st_mode & S_IRUSR));

			if( !bCanRead ) // check groups
			{
				for( int i = 0; i < g_gidCount; i++ )
				{
					if( st.st_gid == g_gids[i] && (st.st_mode & S_IRGRP ) )
					{
						bCanRead = true;
						break;
					}
				}
			}

			if( !bCanRead )
			{
				fprintf(stderr, "%s: have no permission to read file %s\n", g_pModuleName , path);
				continue;
			}

			files[*file_count] = malloc(sizeof(fileinfo_t));

			fileinfo_t *file = files[*file_count];
			file->size = st.st_size;
			strncpy(file->name, path, MAX_PATH);

			file->basename = basename( file->name );
			(*file_count)++;

			if( (*file_count) >= (k*FILE_BASE_COUNT) )
			{
				k <<= 1;
				files = realloc( files, sizeof(fileinfo_t*)*k*FILE_BASE_COUNT);
			}

		}
	}

	if( closedir(dir) == -1 )
		sys_error("closedir failed for %s", cur_path);

	return files;
}

void copy_file( char *from, char *to )
{
	char destname[MAX_PATH];

	FILE *f_from = fopen(from, "r");

	strncpy(destname, to, sizeof(destname));
	if( access( destname, F_OK ) == 0 )
	{
		for(int i = 1; ;i++)
		{
			snprintf(destname, sizeof destname, "%s_%d", to, i);
			if( access( destname, F_OK ) != 0 )
				break;
		}
	}

	if( !f_from )
	{
		sys_error( "fopen failed for file %s", from);
		return;
	}

	static char buffer[BUFFER_SIZE]; // 1 Mbyte buffer

	size_t size = fread(buffer, sizeof(char), sizeof(char)*BUFFER_SIZE, f_from);

	if( size == 0 && ferror(f_from) )
	{
		sys_error("fread failed for file %s", from);
		return;
	}

	FILE *f_to = fopen(destname, "w+");
	if( !f_to )
	{
		sys_error( "fopen failed for file %s", to);
		if( fclose(f_from) == EOF )
			sys_error("fclose failed for %s", to);
		return;
	}

	size_t writed = fwrite(buffer, sizeof(char), size, f_to);
	if( writed == 0 && ferror(f_to) )
	{
		sys_error("fwrite failed for file %s", to);
		return;
	}

	while( (size = fread(buffer, sizeof(char), BUFFER_SIZE, f_from)) != 0 )
		fwrite(buffer, sizeof(char), size, f_to);

	if( fclose(f_from) == EOF )
		sys_error("fclose failed for %s", from);

	if( fclose(f_to) == EOF )
		sys_error("fclose failed for %s", to);

}

int main(int argc, char **argv)
{
	g_pModuleName = basename(argv[0]);
	g_uid = getuid();

	struct passwd* pw = getpwuid(g_uid);

	//GET GROUP COUNT
	getgrouplist(pw->pw_name, pw->pw_gid, NULL, &g_gidCount);
	g_gids = (gid_t*)malloc( sizeof(gid_t) * g_gidCount );

	//GET GROUP LIST
	getgrouplist(pw->pw_name, pw->pw_gid, g_gids, &g_gidCount);


	if( argc < 4 )
	{
		fprintf(stderr, "error: not enough arguments!\n");
		return -1;
	}

	char *dirname = argv[1];
	char *outdir = argv[3];
	int opt = atoi( argv[2] );

	mkdir( outdir, 0755 );

	int file_count = 0;
	fileinfo_t **files = recursive_find(NULL, &file_count, dirname);

	if( files == NULL )
	{
		fprintf(stderr, "error: there is no files in %s directory!\n", dirname);
		return -1;
	}

	if( opt == 1 )
		sort_by_size(files, file_count);
	else if( opt == 2 )
		sort_by_name(files, file_count);

	for( int i = 0; i < file_count; i++ )
	{
		char dest_path[MAX_PATH];
		char *base = basename(files[i]->name);
		snprintf( dest_path, MAX_PATH, "%s/%s", outdir, base);
		copy_file( files[i]->name, dest_path );
		free( files[i] );
	}

	free(files);
	free(g_gids);

	return 0;
}
